//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Server.h"
#include "utils/CanonicalMode.h"
#include "utils/CommandArgs.h"
#include "utils/CSVWriter.h"
#include "utils/Error.h"
#include "utils/Logger.h"
#include "utils/Msg.h"
#include "utils/Screen.h"
#include "utils/StringUtils.h"
#include "db/FileSysDatabase.h"
#include "db/FileSysDBRecord.h"

namespace subsim
{

//-----------------------------------------------------------------------------
const Version SERVER_VERSION("1.0.x");
const std::string ADDRESS_PREFIX("Adress: ");
const std::string BOOTED("booted");
const std::string COMM_ERROR("comm error");
const std::string GAME_FULL("game full");
const std::string INVALID_NAME("invalid name");
const std::string NAME_IN_USE("name in use");
const std::string NAME_TOO_LONG("name too long");
const std::string PLAYER_EXITED("exited");
const std::string PLAYER_PREFIX("Player: ");
const std::string PROTOCOL_ERROR("protocol error");
const std::string INVALID_SUBS("invalid sub data");
const unsigned MAX_PLAYER_NAME_SIZE = 12;

//-----------------------------------------------------------------------------
Version
Server::getVersion() {
  return SERVER_VERSION;
}

//-----------------------------------------------------------------------------
void
Server::showHelp() {
  const std::string progname = CommandArgs::getInstance().getProgramName();
  Screen::print()
      << EL
      << "usage: " << progname << " [OPTIONS]" << EL
      << EL
      << "GENERAL OPTIONS:" << EL
      << "  --help                    Show help and exit" << EL
      << "  -l, --log-level <level>   Set log level: DEBUG, INFO, WARN, ERROR " << EL
      << "  -f, --log-file <file>     Write log messages to given file" << EL
      << EL
      << "CONNECTION OPTIONS:" << EL
      << "  -b, --bind-address <addr> Bind server to given IP address" << EL
      << "  -p, --port <port>         Listen for connections on given port" << EL
      << EL
      << "GAME OPTIONS:" << EL
      << "  -t, --title <title>       Set game title to given value" << EL
      << "  -c, --config <file>       Use given GameConfig file" << EL
      << "  -o, --opt <opt>           Set the given game option" << EL
      << EL
      << "SERVER OPTIONS:" << EL
      << "  -a, --auto-start          Auto start game if max players joined" << EL
      << "  -r, --repeat              Repeat game when done" << EL
      << EL
      << "DATABASE OPTIONS:" << EL
      << "  -d, --db-dir <dir>        Save game stats to given directory" << EL
      << EL << Flush;
}

//-----------------------------------------------------------------------------
bool
Server::init() {
  const CommandArgs& args = CommandArgs::getInstance();

  Screen::get() << args.getProgramName() << " version " << getVersion()
                << EL << Flush;

  if (args.has("--help")) {
    showHelp();
    return false;
  }

  autoStart = args.has({"-a", "--auto-start"});
  repeat    = args.has({"-r", "--repeat"});
  return true;
}

//-----------------------------------------------------------------------------
bool
Server::run() {
  std::string title;
  if (!getGameTitle(title)) {
    return false;
  }

  bool ok = true;
  try {
    CanonicalMode cmode(false);
    UNUSED(cmode);

    game.reset(newGameConfig(), title);
    startListening();

    Coordinate coord;
    while (ok && !game.isFinished()) {
      if (game.isStarted()) {
        printMap(coord.set(1, 1));
      } else {
        printGameInfo(coord.set(1, 1));
      }
      printPlayers(coord);
      printOptions(coord);
      if (waitForInput()) {
        ok = handleUserInput(coord);
      }
    }

    printGameInfo(coord.set(1, 1));
    printPlayers(coord);
    if (game.isAborted()) {
      sendGameResults();
      ok = true; // allow restart
    } else if (game.isFinished()) {
      sendGameResults();
      saveResult();
    }
  }
  catch (const std::exception& e) {
    Logger::printError() << e.what();
    ok = false;
  }

  Screen::get(true) << EL << DefaultColor << Flush;
  close();
  return ok;
}

//-----------------------------------------------------------------------------
std::string
Server::prompt(Coordinate coord,
               const std::string& question,
               const char delim)
{
  CanonicalMode cmode(true);
  UNUSED(cmode);

  Screen::print() << coord << ClearToLineEnd << question << Flush;
  return input.readln(STDIN_FILENO, delim) ? input.getStr() : std::string();
}

//-----------------------------------------------------------------------------
GameConfig Server::newGameConfig() {
  const CommandArgs& args = CommandArgs::getInstance();
  GameConfig config;

  const int count = args.getCount();
  for (int i = 0; (i + 1) < count; ++i) {
    if (args.match(i, {"-c", "--config"})) {
      std::string str = args.get(++i);
      if (str.size()) {
        config.loadFrom(FileSysDBRecord(str, str));
      }
    } else if (args.match(i, {"-o", "--opt"})) {
      std::string str = args.get(++i);
      if (str.size()) {
        config.addSetting(GameSetting::fromMessage(str));
      }
    }
  }

  config.validate();
  return std::move(config);
}

//-----------------------------------------------------------------------------
bool
Server::getGameTitle(std::string& title) {
  title = CommandArgs::getInstance().getStrAfter({"-t", "--title"});
  do {
    if (isEmpty(title)) {
      Screen::print() << "Enter game title [RET=quit] -> " << Flush;
      if (!input.readln(STDIN_FILENO)) {
        return false;
      }
      title = input.getStr();
      if (title.empty()) {
        return false;
      }
    }
    if ((input.getFieldCount() > 1) || contains(title, '|')) {
      Screen::print() << "Title may not contain '|' character" << EL << Flush;
      title.clear();
    } else if (title.size() > 20) {
      Screen::print() << "Title may not exceed 20 characters" << EL << Flush;
      title.clear();
    }
  } while (isEmpty(title));
  return true;
}

//-----------------------------------------------------------------------------
bool
Server::isServerHandle(const int handle) const {
  return ((handle >= 0) && (handle == socket.getHandle()));
}

//-----------------------------------------------------------------------------
bool
Server::isUserHandle(const int handle) const {
  return ((handle >= 0) && (handle == STDIN_FILENO));
}

//-----------------------------------------------------------------------------
bool
Server::isValidPlayerName(const std::string& name) const {
  return ((name.size() > 1) && isalpha(name[0]) &&
      !iEqual(name, "server") &&
      !iEqual(name, "all") &&
      !iEqual(name, "you") &&
      !iEqual(name, "new") &&
      !iEqual(name, "me") &&
      !containsAny(name, "<>[]{}()|\n\r\t"));
}

//-----------------------------------------------------------------------------
bool
Server::sendGameInfo(Player& recipient) {
  const GameConfig& config = game.getConfig();
  if (!send(recipient, config.toMessage(getVersion(), game.getTitle()))) {
    return false;
  }
  for (const GameSetting& setting : config.getCustomSettings()) {
    if (!send(recipient, setting.toMessage())) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Server::send(Player& recipient, const std::string& msg,
             const bool removeOnFailure)
{
  if (!recipient.send(msg)) {
    if (removeOnFailure) {
      removePlayer(recipient, COMM_ERROR);
    }
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Server::waitForInput(const int timeout) {
  std::set<int> ready;
  if (!input.waitForData(ready, timeout)) {
    return false;
  }

  bool userInput = false;
  for (const int handle : ready) {
    if (isServerHandle(handle)) {
      addPlayerHandle();
    } else if (isUserHandle(handle)) {
      userInput = true;
    } else {
      handlePlayerInput(handle);
    }
  }
  return userInput;
}

//-----------------------------------------------------------------------------
void
Server::addPlayerHandle() {
  PlayerPtr player = std::make_shared<Player>("new", socket.accept());
  if (!player->isConnected()) {
    Logger::debug() << "no new connetion from accept"; // not an error
    return;
  }

  if (game.getPlayer(player->handle())) {
    throw Error(Msg() << "Duplicate handle accepted: " << (*player));
  }

  if (blackList.count(ADDRESS_PREFIX + player->getAddress())) {
    Logger::debug() << (*player) << " address is blacklisted";
    return;
  }

  if (sendGameInfo(*player)) {
    input.addHandle(player->handle(), player->getAddress());
    stagedPlayers[player->handle()] = player;
  }
}

//-----------------------------------------------------------------------------
void
Server::beginGame() {
  stopListening();

  std::map<unsigned, std::string> errs = game.start();
  for (auto it = errs.begin(); it != errs.end(); ++it) {
    removePlayer(static_cast<int>(it->first), it->second);
  }
}

//-----------------------------------------------------------------------------
void
Server::blacklistAddress(Coordinate coord) {
  std::string str = prompt(coord, "Enter IP address to blacklist -> ");
  if (str.size()) {
    blackList.insert(ADDRESS_PREFIX + str);
    for (auto& player : game.playersFromAddress(str)) {
      removePlayer(*player);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::blacklistPlayer(Coordinate coord) {
  if (!game.getPlayerCount()) {
    return;
  }

  std::string name;
  name = prompt(coord, "Enter name or number of player to blacklist -> ");
  if (name.size()) {
    PlayerPtr player = game.getPlayer(name);
    if (!player) {
      player = game.getPlayer(toInt32(name, -1));
    }
    if (player) {
      blackList.insert(PLAYER_PREFIX + player->getName());
      removePlayer(*player);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::bootPlayer(Coordinate coord) {
  if (!game.getPlayerCount()) {
    return;
  }

  std::string name;
  name = prompt(coord, "Enter name or number of player to boot -> ");
  if (name.size()) {
    PlayerPtr player = game.getPlayer(name);
    if (!player) {
      player = game.getPlayer(toInt32(name, -1));
    }
    if (player) {
      removePlayer((*player), BOOTED);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::clearBlacklist(Coordinate coord) {
  if (blackList.empty()) {
    return;
  }

  Screen::print() << coord << ClearToScreenEnd << "Blacklist:";
  coord.south().setX(3);
  for (auto& str : blackList) {
    Screen::print() << coord.south() << str;
  }

  std::string s = prompt(coord.south(2).setX(1), "Clear Blacklist? [y/N] -> ");
  if (iStartsWith(s, 'Y')) {
    blackList.clear();
  }
}

//-----------------------------------------------------------------------------
void
Server::clearScreen() {
  Screen::get(true).clear().flush();
}

//-----------------------------------------------------------------------------
void
Server::close() {
  for (const auto& player : game.getPlayers()) {
    input.removeHandle(player->handle());
  }
  game.clearPlayers();

  for (const auto& pair : stagedPlayers) {
    input.removeHandle(pair.second->handle());
  }
  stagedPlayers.clear();

  stopListening();
}

//-----------------------------------------------------------------------------
void
Server::handlePlayerInput(const int handle) {
  if (!input.readln(handle)) {
    removePlayer(handle);
  } else {
    std::string err;
    std::string str = input.getStr();
    if (str == "J") {
      joinGame(handle);
    } else if (!game.addCommand(handle, input, err)) {
      removePlayer(handle, err);
    }
  }

  if (game.allCommandsReceived()) {
    std::map<unsigned, std::string> errs = game.executeTurn();
    for (auto it = errs.begin(); it != errs.end(); ++it) {
      removePlayer(static_cast<int>(it->first), it->second);
    }
  }
}

//-----------------------------------------------------------------------------
bool
Server::handleUserInput(Coordinate coord) {
  char ch = 0;
  if (input.readKey(STDIN_FILENO, ch) == KeyChar) {
    switch (toupper(ch)) {
    case 'A': blacklistAddress(coord); break;
    case 'B': bootPlayer(coord);       break;
    case 'C': clearBlacklist(coord);   break;
    case 'P': blacklistPlayer(coord);  break;
    case 'Q': return !quitGame(coord);
    case 'R': clearScreen();           break;
    case 'S': startGame(coord);        break;
    case 'V': viewMap();               break;
    default:
      break;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
void
Server::joinGame(const int handle) {
  auto it = stagedPlayers.find(handle);
  if (it == stagedPlayers.end()) {
    removePlayer(handle, PROTOCOL_ERROR);
    return;
  }

  const unsigned maxPlayers = game.getConfig().getMaxPlayers();
  PlayerPtr player = it->second;
  if (!player) {
    throw Error("Server.joinGame() null player");
  } else if (game.getPlayer(player->handle())) {
    throw Error(Msg() << "duplicate player handle (" << player->handle()
                << ") in join command!");
  } else if (game.isStarted()) {
    throw Error("Server.joinGame() game already started");
  } else if (maxPlayers && (game.getPlayerCount() >= maxPlayers)) {
    removePlayer((*player), GAME_FULL);
    return;
  }

  const std::string playerName = input.getStr(1);
  if (playerName.empty()) {
    removePlayer((*player), PROTOCOL_ERROR);
    return;
  } else if (blackList.count(PLAYER_PREFIX + playerName)) {
    removePlayer(*player);
    return;
  } else if (!isValidPlayerName(playerName)) {
    removePlayer((*player), INVALID_NAME);
    return;
  } else if (playerName.size() > MAX_PLAYER_NAME_SIZE) {
    removePlayer((*player), NAME_TOO_LONG);
    return;
  } else if (game.getPlayer(playerName)) {
    removePlayer((*player), NAME_IN_USE);
    return;
  }

  removeStagedPlayer(player->handle());
  player->setName(playerName);

  const std::string err = game.addPlayer(player, input);
  if (err.size()) {
    removePlayer((*player), err);
    return;
  }

  // send confirmation to joining Player
  CSVWriter joinMsg = Msg('J') << playerName;
  send((*player), joinMsg);

  // start the game if max player count reached and autoStart enabled
  if (maxPlayers && (game.getPlayerCount() == maxPlayers)) {
    if (autoStart) {
      beginGame();
    } else {
      stopListening();
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::printGameInfo(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd;
  game.printSummary(coord);
}

//-----------------------------------------------------------------------------
void
Server::printMap(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd;
  game.getMap().print(coord);
}

//-----------------------------------------------------------------------------
void
Server::printOptions(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd
                  << "(Q)uit, (R)edraw, Blacklist (A)ddress";

  if (blackList.size()) {
    Screen::print() << ", (C)lear blacklist";
  }

  if (game.getPlayerCount()) {
    Screen::print() << coord.south() << "(B)oot Player, Blacklist (P)layer";
  }

  if (!game.isStarted()) {
    if (game.canStart()) {
      Screen::print() << coord.south() << "(V)iew Map, (S)tart Game";
    } else {
      Screen::print() << coord.south() << "(V)iew Map";
    }
  }

  Screen::print() << " -> " << Flush;
}

//-----------------------------------------------------------------------------
void
Server::printPlayers(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd
                  << "Players Joined : " << game.getPlayerCount()
                  << (game.isStarted() ? " (In Progress)" : " (Not Started)");

  coord.south().setX(3);

  for (auto& player : game.getPlayers()) {
    Screen::print() << coord.south() << player->summary(game.isStarted());
  }

  Screen::print() << coord.south().setX(1);
  if (game.getPlayerCount()) {
    Screen::print() << coord.south();
  }
}

//-----------------------------------------------------------------------------
bool
Server::quitGame(Coordinate coord) {
  std::string msg;
  if (game.isStarted()) {
    msg = "Quit Game?";
  } else {
    msg = "Terminate Server?";
  }
  std::string s = prompt(coord, (msg + " [y/N] -> "));
  if (iStartsWith(s, 'Y')) {
    if (game.isStarted()) {
      game.abort();
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void
Server::removePlayer(const int handle, const std::string& msg) {
  PlayerPtr player = game.getPlayer(handle);
  auto it = stagedPlayers.find(handle);
  if (it != stagedPlayers.end()) {
    if (player) {
      throw Error(Msg() << "duplicated player handle: " << handle);
    }
    removePlayer(*(it->second));
  } else if (player) {
    removePlayer((*player), msg);
  } else {
    throw Error(Msg() << "Server::removePlayer() no player for handle "
                << handle << " found");
  }

  if (game.getPlayer(handle) || stagedPlayers.count(handle)) {
    throw Error(Msg() << "failed to remove player handle: " << handle);
  }
}

//-----------------------------------------------------------------------------
void
Server::removePlayer(Player& player, const std::string& msg) {
  const std::string name = player.getName();
  const int handle = player.handle();
  input.removeHandle(handle);

  if (msg.size() && (msg != COMM_ERROR)) {
    send(player, msg, false);
  }

  auto it = stagedPlayers.find(handle);
  if (it != stagedPlayers.end()) {
    if (game.getPlayer(handle) || game.getPlayer(name)) {
      throw Error(Msg() << player << " in game and staged player lists");
    }
    stagedPlayers.erase(it);
  } else {
    game.removePlayer(handle);
  }

  if (!game.isStarted() && !socket.isOpen() &&
      (game.getPlayerCount() < game.getConfig().getMaxPlayers()))
  {
    startListening();
  }
}

//-----------------------------------------------------------------------------
void
Server::removeStagedPlayer(const int handle) {
  auto it = stagedPlayers.find(handle);
  if (it != stagedPlayers.end()) {
    stagedPlayers.erase(it);
  }
}

//-----------------------------------------------------------------------------
void
Server::saveResult() {
  const CommandArgs& args = CommandArgs::getInstance();
  FileSysDatabase db;
  db.open(args.getStrAfter({"-d", "--db-dir"}));
  game.saveResults(db);
  db.sync();
}

//-----------------------------------------------------------------------------
void
Server::sendGameResults() {
  CSVWriter finishMessage = Msg('F')
      << game.getPlayerCount()
      << game.getTurnNumber()
      << (game.isAborted() ? "aborted" : "finished");

  // get local list of players (in case any drop out while sending)
  std::vector<PlayerPtr> players = game.getPlayers();

  // send finish message and player result messages to all Players
  for (auto& recipient : players) {
    if (recipient->isConnected()) {
      send((*recipient), finishMessage);
      for (auto& player : players) {
        send((*recipient), Msg('P') << player->getName() << player->getScore());
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::sendToAll(const std::string& msg) {
  // get local list of players (in case any drop out while sending)
  std::vector<PlayerPtr> players = game.getPlayers();

  for (auto& player : players) {
    if (player->isConnected()) {
      send((*player), msg);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::startGame(Coordinate coord) {
  if (game.canStart() && !game.isStarted()) {
    std::string str = prompt(coord, "Start Game? [y/N] -> ");
    if (iStartsWith(str, 'Y')) {
      beginGame();
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::startListening() {
  const int playerCount = static_cast<int>(game.getPlayerCount());
  int backlog = static_cast<int>(game.getConfig().getMaxPlayers());
  if (backlog <= 0) {
    backlog = 10;
  } else {
    // backlog = (maxPlayers - playerCount + 1)
    backlog = std::max<int>(1, (backlog - playerCount + 1));
  }

  const CommandArgs& args = CommandArgs::getInstance();
  const std::string bindAddress = args.getStrAfter({"-b", "--bind-address"});
  int bindPort = args.getIntAfter({"-p", "--port"}, DEFAULT_PORT);

  socket.listen(bindAddress, bindPort, backlog);
  input.addHandle(socket.getHandle());
}

//-----------------------------------------------------------------------------
void
Server::stopListening() {
  if (socket) {
    input.removeHandle(socket.getHandle());
    socket.close();
  }
}

//-----------------------------------------------------------------------------
void
Server::viewMap() {
  if (!game.isStarted()) {
    Coordinate coord(1, 1);
    printMap(coord);
    prompt(coord, "RET=continue");
  }
}

} // namespace subsim
