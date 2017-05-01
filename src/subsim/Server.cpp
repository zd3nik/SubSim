//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Server.h"
#include "CanonicalMode.h"
#include "CommandArgs.h"
#include "CSVWriter.h"
#include "Logger.h"
#include "Msg.h"
#include "Screen.h"
#include "StringUtils.h"
#include "Error.h"
#include "db/FileSysDatabase.h"
#include "db/FileSysDBRecord.h"

namespace subsim
{

//-----------------------------------------------------------------------------
const Version SERVER_VERSION("2.0.x");
const std::string ADDRESS_PREFIX("Adress: ");
const std::string BOOTED("booted");
const std::string COMM_ERROR("comm error");
const std::string GAME_ABORTED("game aborted");
const std::string GAME_FULL("game is full");
const std::string GAME_STARETD("game is already started");
const std::string INVALID_BOARD("invalid board");
const std::string INVALID_NAME("E|invalid name");
const std::string NAME_IN_USE("E|name in use");
const std::string NAME_TOO_LONG("E|name too long");
const std::string PLAYER_EXITED("exited");
const std::string PLAYER_PREFIX("Player: ");
const std::string PROTOCOL_ERROR("protocol error");

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
      << "  -q, --quiet               No screen updates during game" << EL
      << EL
      << "CONNECTION OPTIONS:" << EL
      << "  -b, --bind-address <addr> Bind server to given IP address" << EL
      << "  -p, --port <port>         Listen for connections on given port" << EL
      << EL
      << "BOARD OPTIONS:" << EL
      << "  -c, --config <file>       Use given board configuration file" << EL
      << "  --width <count>           Set board width" << EL
      << "  --height <count>          Set board height" << EL
      << EL
      << "GAME OPTIONS:" << EL
      << "  -t, --title <title>       Set game title to given value" << EL
      << "  -a, --auto-start          Auto start game if max players joined" << EL
      << "  -r, --repeat              Repeat game when done" << EL
      << "  --min <players>           Set minimum number of players" << EL
      << "  --max <players>           Set maximum number of players" << EL
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

  quietMode = args.has({"-q", "--quiet"});
  autoStart = args.has({"-a", "--auto-start"});
  repeat    = args.has({"-r", "--repeat"});

  game.clear();
  return true;
}

//-----------------------------------------------------------------------------
bool
Server::run() {
  Configuration config = newGameConfig();
  if (!config) {
    return false;
  }

  std::string title;
  if (!getGameTitle(title)) {
    return false;
  }

  game.clear().setConfiguration(config).setTitle(title);
  startListening(config.getMaxPlayers() + 2);

  bool ok = true;
  try {
    CanonicalMode cmode(false);
    UNUSED(cmode);

    Coordinate coord;
    Coordinate quietCoord;

    while (ok && !game.isFinished()) {
      if (!quietMode || !quietCoord) {
        printGameInfo(coord.set(1, 1));
        printPlayers(coord);
        printOptions(coord);
      }
      if (quietMode && !quietCoord && game.isStarted() && !game.isFinished()) {
        quietCoord.set(coord);
      }
      if (waitForInput()) {
        ok = handleUserInput(coord);
        quietCoord.clear();
      }
    }

    printGameInfo(coord.set(1, 1));
    printPlayers(coord);
    if (!ok) {
      sendToAll(GAME_ABORTED);
      if (game.isStarted()) {
        game.clear();
        ok = true;
      }
    } else if (game.isFinished() && !game.isAborted()) {
      sendGameResults();
      saveResult();
    }
  }
  catch (const std::exception& e) {
    Logger::printError() << e.what();
    ok = false;
  }

  close();

  Screen::get(true) << EL << DefaultColor << Flush;
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
Configuration
Server::newGameConfig() {
  Configuration config = Configuration::getDefaultConfiguration();
  const CommandArgs& args = CommandArgs::getInstance();

  std::string str = args.getStrAfter({"-c", "--config"});
  if (str.size()) {
    config.loadFrom(FileSysDBRecord(str, str));
  }

  str = args.getStrAfter("--min");
  if (str.size()) {
    unsigned val = toUInt32(str);
    if (val < 2) {
      throw Error(Msg() << "Invalid --min value: " << str);
    } else {
      config.setMaxPlayers(val);
    }
  }

  str = args.getStrAfter("--max");
  if (str.size()) {
    unsigned val = toUInt32(str);
    if ((val < 2) || (config.getMinPlayers() > val)) {
      throw Error(Msg() << "Invalid --max value: " << str);
    } else {
      config.setMaxPlayers(val);
    }
  }

  str = args.getStrAfter("--width");
  if (str.size()) {
    unsigned val = toUInt32(str);
    if (val < 8) {
      throw Error(Msg() << "Invalid --width value: " << str);
    } else {
      config.setBoardSize(val, config.getBoardHeight());
    }
  }

  str = args.getStrAfter("--height");
  if (str.size()) {
    unsigned val = toUInt32(str);
    if (val < 8) {
      throw Error(Msg() << "Invalid --height value: " << str);
    } else {
      config.setBoardSize(config.getBoardWidth(), val);
    }
  }

  return config;
}

//-----------------------------------------------------------------------------
bool
Server::getGameTitle(std::string& title) {
  title = CommandArgs::getInstance().getStrAfter({"-t", "--title"});
  while (title.empty()) {
    Screen::print() << "Enter game title [RET=quit] -> " << Flush;
    if (!input.readln(STDIN_FILENO)) {
      return false;
    }
    title = input.getStr();
    if (title.empty()) {
      return false;
    }
    if (input.getFieldCount() > 1) {
      Screen::print() << "Title may not contain '|' character" << EL << Flush;
      title.clear();
    } else if (title.size() > 20) {
      Screen::print() << "Title may not exceed 20 characters" << EL << Flush;
      title.clear();
    }
  }
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
      !containsAny(name, "<>[]{}()"));
}

//-----------------------------------------------------------------------------
bool
Server::sendBoard(Player& recipient, const Player& board) {
  return send(recipient, Msg('B')
              << board.getName()
              << board.getStatus()
              << board.maskedDescriptor()
              << board.getScore()
              << board.getSkips());
}

//-----------------------------------------------------------------------------
bool
Server::sendGameInfo(Player& recipient) {
  const Configuration& config = game.getConfiguration();
  CSVWriter msg = Msg('G')
      << getVersion()
      << config.getName()
      << (game.isStarted() ? 'Y' : 'N')
      << config.getMinPlayers()
      << config.getMaxPlayers()
      << game.getBoardCount()
      << config.getPointGoal()
      << config.getBoardWidth()
      << config.getBoardHeight()
      << config.getShipCount();

  for (const Ship& ship : config) {
    msg << ship.toString();
  }

  return send(recipient, msg.toString());
}

//-----------------------------------------------------------------------------
bool
Server::sendYourBoard(Player& recipient) {
  std::string desc = recipient.getDescriptor();
  for (char& ch : desc) {
    ch = Ship::unHit(ch);
  }
  return send(recipient, Msg('Y') << desc);
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
  const Configuration& config = game.getConfiguration();
  auto board = std::make_shared<Player>("new", config, socket.accept());
  if (!board->isConnected()) {
    Logger::debug() << "no new connetion from accept"; // not an error
    return;
  }

  if (game.hasBoard(board->handle())) {
    throw Error(Msg() << "Duplicate handle accepted: " << (*board));
  }

  if (blackList.count(ADDRESS_PREFIX + board->getAddress())) {
    Logger::debug() << (*board) << " address is blacklisted";
    return;
  }

  if (sendGameInfo(*board) && game.hasOpenBoard()) {
    input.addHandle(board->handle(), board->getAddress());
    newBoards[board->handle()] = board;
  }
}

//-----------------------------------------------------------------------------
void
Server::blacklistAddress(Coordinate coord) {
  std::string str = prompt(coord, "Enter IP address to blacklist -> ");
  if (str.size()) {
    blackList.insert(ADDRESS_PREFIX + str);
    for (auto& board : game.boardsForAddress(str)) {
      removePlayer((*board), BOOTED);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::blacklistPlayer(Coordinate coord) {
  if (!game.getBoardCount()) {
    return;
  }

  std::string user;
  user = prompt(coord, "Enter name or number of player to blacklist -> ");
  if (user.size()) {
    auto board = game.boardForPlayer(user, false);
    if (board) {
      blackList.insert(PLAYER_PREFIX + board->getName());
      removePlayer((*board), BOOTED);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::bootPlayer(Coordinate coord) {
  if (!game.getBoardCount()) {
    return;
  }
  std::string user;
  user = prompt(coord, "Enter name or number of player to boot -> ");
  if (user.size()) {
    auto board = game.boardForPlayer(user, false);
    if (board) {
      removePlayer((*board), BOOTED);
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
  for (auto& board : game.getBoards()) {
    input.removeHandle(board->handle());
  }
  game.clear();

  for (auto& pair : newBoards) {
    input.removeHandle(pair.second->handle());
  }
  newBoards.clear();

  if (socket) {
    input.removeHandle(socket.getHandle());
    socket.close();
  }
}

//-----------------------------------------------------------------------------
void
Server::handlePlayerInput(const int handle) {
  auto it = newBoards.find(handle);
  auto board = (it == newBoards.end())
      ? game.boardForHandle(handle)
      : it->second;

  if (!board) {
    throw Error(Msg() << "Unknown player handle: " << handle);
  }

  if (!input.readln(handle)) {
    Logger::warn() << "Disconnecting " << (*board);
    removePlayer(*board);
    return;
  }

  std::string str = input.getStr();
  if (str.size() == 1) {
    switch (str[0]) {
    case 'G': sendGameInfo(*board); return;
    case 'J': joinGame(board);      return;
    case 'K': skipTurn(*board);     return;
    case 'L': leaveGame(*board);    return;
    case 'M': sendMessage(*board);  return;
    case 'P': ping(*board);         return;
    case 'S': shoot(*board);        return;
    case 'T': setTaunt(*board);     return;
    default:
      break;
    }
  }

  Logger::debug() << "Invalid message(" << input.getLine() << ") from "
                  << (*board);

  send((*board), PROTOCOL_ERROR);
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
    case 'K': skipBoard(coord);        break;
    case 'M': sendMessage(coord);      break;
    case 'P': blacklistPlayer(coord);  break;
    case 'Q': return !quitGame(coord);
    case 'R': clearScreen();           break;
    case 'S': startGame(coord);        break;
    default:
      break;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
void
Server::joinGame(BoardPtr& joiner) {
  if (!joiner) {
    throw Error("Server.joinGame() null board");
  }

  const Configuration& config = game.getConfiguration();
  const std::string playerName = input.getStr(1);
  const std::string shipDescriptor = input.getStr(2);

  if (game.hasBoard(joiner->handle())) {
    throw Error(Msg() << "duplicate handle (" << joiner->handle()
                << ") in join command!");
  } else if (playerName.empty()) {
    removePlayer((*joiner), PROTOCOL_ERROR);
  } else if (blackList.count(PLAYER_PREFIX + playerName)) {
    removePlayer((*joiner), BOOTED);
  } else if (!game.hasOpenBoard()) {
    removePlayer((*joiner), GAME_FULL);
  } else if (!isValidPlayerName(playerName)) {
    send((*joiner), INVALID_NAME);
  } else if (playerName.size() > config.getBoardWidth()) {
    send((*joiner), NAME_TOO_LONG);
  } else if (game.isStarted()) {
    auto existingBoard = game.boardForPlayer(playerName, true);
    if (existingBoard) {
      if (existingBoard->isConnected()) {
        send((*joiner), NAME_IN_USE);
      } else {
        rejoinGame(existingBoard->stealConnectionFrom(std::move(*joiner)));
      }
    } else {
      removePlayer((*joiner), GAME_STARETD);
    }
  } else if (game.hasBoard(playerName)) {
    removePlayer((*joiner), NAME_IN_USE);
  } else if (!config.isValidShipDescriptor(shipDescriptor) ||
             !joiner->updateDescriptor(shipDescriptor))
  {
    removePlayer((*joiner), INVALID_BOARD);
  } else {
    removeNewBoard(joiner->handle());
    joiner->setName(playerName);
    game.addBoard(joiner);

    // send confirmation to joining board
    CSVWriter joinMsg = Msg('J') << playerName;
    send((*joiner), joinMsg);

    // send name of other players to joining board
    for (auto& board : game.getBoards()) {
      if ((board->handle() != joiner->handle()) &&
          !send((*joiner), Msg('J') << board->getName()))
      {
        return;
      }
    }

    // let other players know playerName has joined
    for (auto& recipient : game.getBoards()) {
      if (recipient->isConnected() &&
          (recipient->handle() != joiner->handle()))
      {
        send((*recipient), joinMsg);
      }
    }

    // start the game if max player count reached and autoStart enabled
    if (autoStart && !game.isStarted() &&
        (game.getBoardCount() == config.getMaxPlayers()) &&
        game.start(true))
    {
      sendStart();
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::leaveGame(Player& board) {
  std::string reason = input.getStr(1);
  removePlayer(board, reason);
}

//-----------------------------------------------------------------------------
void
Server::nextTurn() {
  if (game.nextTurn()) {
    auto toMove = game.boardToMove();
    if (toMove) {
      sendToAll(Msg('N') << toMove->getName());
    } else {
      throw Error("Failed to get board to move");
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::ping(Player& board) {
  std::string msg = input.getLine(false);
  if (msg.length()) {
    send(board, msg);
  } else {
    removePlayer(board, PROTOCOL_ERROR);
  }
}

//-----------------------------------------------------------------------------
void
Server::printGameInfo(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd;
  game.getConfiguration().print(coord);
  Screen::print() << coord.south(1);
}

//-----------------------------------------------------------------------------
void
Server::printOptions(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd
                  << "(Q)uit, (R)edraw, (M)essage, Blacklist (A)ddress";

  if (blackList.size()) {
    Screen::print() << ", (C)lear blacklist";
  }

  if (game.getBoardCount()) {
    Screen::print() << coord.south()
                    << "(B)oot Player, Blacklist (P)layer, S(k)ip Player Turn";
  }

  if (game && !game.isStarted()) {
    Screen::print() << coord.south() << "(S)tart Game";
  }

  Screen::print() << " -> " << Flush;
}

//-----------------------------------------------------------------------------
void
Server::printPlayers(Coordinate& coord) {
  Screen::print() << coord << ClearToScreenEnd
                  << "Players Joined : " << game.getBoardCount()
                  << (game.isStarted() ? " (In Progress)" : " (Not Started)");

  coord.south().setX(3);

  int n = 0;
  for (auto& board : game.getBoards()) {
    Screen::print() << coord.south() << board->summary(++n, game.isStarted());
  }

  Screen::print() << coord.south(2).setX(1);
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
    game.abort();
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void
Server::rejoinGame(Player& joiner) {
  removeNewBoard(joiner.handle());
  joiner.setStatus("");

  // send confirmation and yourboard info to rejoining player
  if (!send(joiner, Msg('J') << joiner.getName()) ||
      !sendYourBoard(joiner))
  {
    return;
  }

  // send details about other players to rejoining player
  CSVWriter startMsg = Msg('S');
  for (auto& board : game.getBoards()) {
    if (board->handle() != joiner.handle()) {
      if (!send(joiner, Msg('J') << board->getName()) ||
          !sendBoard(joiner, (*board)))
      {
        return;
      }
    }
    startMsg << board->getName(); // collect boards in startMsg for use below
  }

  // send start game message to rejoining player
  if (!send(joiner, startMsg)) {
    return;
  }

  // let rejoining player know who's turn it is
  auto toMove = game.boardToMove();
  if (!toMove) {
    throw Error("Board to move unknown!");
  } else if (!send(joiner, Msg('N') << toMove->getName())) {
    return;
  }

  // send rejoining player's board to everybody
  for (auto& recipient : game.getBoards()) {
    if (recipient->isConnected()) {
      sendBoard((*recipient), joiner);
    }
  }

  // send message to all that rejoining player player has reconnected
  sendToAll(Msg('M') << "" << (joiner.getName() + " reconnected"));
}

//-----------------------------------------------------------------------------
void
Server::removeNewBoard(const int handle) {
  auto it = newBoards.find(handle);
  if (it != newBoards.end()) {
    newBoards.erase(it);
  }
}

//-----------------------------------------------------------------------------
void
Server::removePlayer(Player& board, const std::string& msg) {
  input.removeHandle(board.handle());

  if (msg.size() && (msg != COMM_ERROR)) {
    send(board, msg, false);
  }

  auto it = newBoards.find(board.handle());
  if (it != newBoards.end()) {
    if (game.hasBoard(board.handle()) || game.hasBoard(board.getName())) {
      throw Error(Msg() << board << " in game boards and new boards array");
    }
    newBoards.erase(it);
    return;
  }

  if (game.isStarted()) {
    game.disconnectBoard(board.getName(), msg);
  } else {
    game.removeBoard(board.getName());
  }

  for (auto& recipient : game.getBoards()) {
    if (recipient->isConnected() &&
        (recipient->handle() != board.handle()))
    {
      if (game.isStarted()) {
        sendBoard((*recipient), board);
      } else {
        send((*recipient), Msg('L') << board.getName() << msg);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::saveResult() {
  if (game.isStarted() && game.isFinished()) {
    const CommandArgs& args = CommandArgs::getInstance();
    FileSysDatabase db;
    db.open(args.getStrAfter({"-d", "--db-dir"}));
    game.saveResults(db);
    db.sync();
  }
}

//-----------------------------------------------------------------------------
void
Server::sendBoardToAll(const Player& board) {
  for (auto& recipient : game.getBoards()) {
    if (recipient->isConnected()) {
      sendBoard((*recipient), board);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::sendGameResults() {
  CSVWriter finishMessage = Msg('F')
      << ((game.isFinished() && !game.isAborted()) ? "finished" : "aborted")
      << game.getTurnCount()
      << game.getBoardCount();

  // send finish message to all boards
  auto boards = game.getBoards();
  for (auto& recipient : boards) {
    if (recipient->isConnected()) {
      send((*recipient), finishMessage);
    }
  }

  // sort boards by score, descending
  std::stable_sort(boards.begin(), boards.end(),
                   [](const BoardPtr& a, const BoardPtr& b) {
    return (a->getScore() > b->getScore());
  }
  );

  // send sorted result messages to all boards (N x N)
  for (auto& recipient : boards) {
    for (auto& board : boards) {
      send((*recipient), Msg('R')
           << board->getName()
           << board->getScore()
           << board->getSkips()
           << board->getTurns()
           << board->getStatus());
    }
  }

  // disconnect all boards
  for (auto& board : boards) {
    removePlayer(*board);
  }
}

//-----------------------------------------------------------------------------
void
Server::sendMessage(Player& sender) {
  // TODO blacklist sender if too many messages too rapidly
  const std::string playerName = input.getStr(1);
  const std::string message    = input.getStr(2);
  if (message.size()) {
    CSVWriter msg = Msg('M') << sender.getName() << message;
    if (playerName.empty()) {
      msg << "All";
    }
    for (auto& recipient : game.getBoards()) {
      if (recipient->isConnected() &&
          (recipient->handle() != sender.handle()) &&
          (playerName.empty() || (recipient->getName() == playerName)))
      {
        send((*recipient), msg);
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::sendMessage(Coordinate coord) {
  if (!game.getBoardCount()) {
    return;
  }

  std::string str;
  str = prompt(coord, "Enter recipient name or number [RET=All] -> ");
  auto recipient = game.boardForPlayer(str, false);
  if (str.size() && !recipient) {
    Logger::printError() << "Unknown player: " << str;
    return;
  }

  str = prompt(coord, "Enter message [RET=Abort] -> ");
  if (str.size()) {
    if (recipient) {
      send((*recipient), Msg('M') << "" << str);
    } else {
      sendToAll(Msg('M') << "" << str);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::sendStart() {
  auto toMove = game.boardToMove();
  if (!toMove) {
    throw Error("No board set to move");
  }

  // send all boards to all players (N x N)
  CSVWriter startMsg = Msg('S');
  for (auto& board : game.getBoards()) {
    sendBoardToAll(*board);
    startMsg << board->getName(); // add board/player name to 'S' message
  }

  // send 'S' (start) and 'N' (next turn) messages to all boards
  CSVWriter nextTurnMsg = Msg('N') << toMove->getName();
  for (auto& recipient : game.getBoards()) {
    if (send((*recipient), startMsg)) {
      send((*recipient), nextTurnMsg);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::sendToAll(const std::string& msg) {
  for (auto& recipient : game.getBoards()) {
    if (recipient->isConnected()) {
      send((*recipient), msg);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::setTaunt(Player& board) {
  std::string type  = input.getStr(1);
  std::string taunt = input.getStr(2);
  if (iEqual(type, "hit")) {
    if (taunt.empty()) {
      board.clearHitTaunts();
    } else {
      board.addHitTaunt(taunt);
    }
  } else if (iEqual(type, "miss")) {
    if (taunt.empty()) {
      board.clearMissTaunts();
    } else {
      board.addMissTaunt(taunt);
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::shoot(Player& shooter) {
  if (!game.isStarted()) {
    send(shooter, "M||game hasn't started");
    return;
  } else if (game.isFinished()) {
    send(shooter, "M||game is already finished");
    return;
  }

  auto toMove = game.boardToMove();
  if (!toMove) {
    throw Error("Board to move unknown!");
  } else if (shooter.getName() != toMove->getName()) {
    send(shooter, "M||it is not your turn!");
    return;
  }

  const std::string targetPlayer = input.getStr(1);
  auto target = game.boardForPlayer(targetPlayer, true);
  if (!target) {
    send(shooter, "M||invalid target player name");
    return;
  } else if (shooter.getName() == target->getName()) {
    send(shooter, "M||don't shoot at yourself stupid!");
    return;
  }

  Coordinate coord(input.getUInt(2), input.getUInt(3));
  const char id = target->shootSquare(coord);
  if (!id) {
    send(shooter, "M||illegal coordinates");
  } else if (Ship::isHit(id) || Ship::isMiss(id)) {
    send(shooter, "M||that spot has already been shot");
  } else {
    shooter.incTurns();
    if (Ship::isValidID(id)) {
      shooter.incScore();
      sendToAll(Msg('H') << shooter.getName() << target->getName() << coord);
      if (target->hasHitTaunts()) {
        send(shooter, Msg('M') << target->getName() << target->nextHitTaunt());
      }
      sendBoardToAll(shooter);
    } else if (target->hasMissTaunts()) {
      send(shooter, Msg('M') << target->getName() << target->nextMissTaunt());
    }
    sendBoardToAll(*target);
    nextTurn();
  }
}

//-----------------------------------------------------------------------------
void
Server::skipBoard(Coordinate coord) {
  auto toMove = game.boardToMove();
  if (!toMove) {
    return;
  }

  std::string str;
  str = prompt(coord, ("Skip " + toMove->getName() + "'s turn? [y/N] -> "));
  if (iStartsWith(str, 'Y')) {
    str = prompt(coord, "Enter reason [RET=Abort] -> ");
    if (str.size()) {
      toMove->incSkips();
      toMove->incTurns();
      sendToAll(Msg('K') << toMove->getName() << str);
      nextTurn();
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::skipTurn(Player& board) {
  if (!game.isStarted()) {
    send(board, "M||game hasn't started");
    return;
  } else if (game.isFinished()) {
    send(board, "M||game is already finished");
    return;
  }

  auto toMove = game.boardToMove();
  if (!toMove) {
    throw Error("Board to move unknown!");
  } else if (board.getName() != toMove->getName()) {
    send(board, "M||it is not your turn!");
  } else {
    board.incSkips();
    board.incTurns();
    nextTurn();
  }
}

//-----------------------------------------------------------------------------
void
Server::startGame(Coordinate coord) {
  if (game && !game.isStarted()) {
    std::string str = prompt(coord, "Start Game? [y/N] -> ");
    if (iStartsWith(str, 'Y') && game.start(true)) {
      sendStart();
    }
  }
}

//-----------------------------------------------------------------------------
void
Server::startListening(const int backlog) {
  const CommandArgs& args = CommandArgs::getInstance();
  const std::string bindAddress = args.getStrAfter({"-b", "--bind-address"});
  int bindPort = args.getIntAfter({"-p", "--port"}, DEFAULT_PORT);

  socket.listen(bindAddress, bindPort, backlog);
  input.addHandle(socket.getHandle());
}

} // namespace subsim
