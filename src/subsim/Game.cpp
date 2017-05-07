//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Game.h"
#include "utils/Error.h"
#include "utils/Logger.h"
#include "utils/Msg.h"
#include "utils/Screen.h"
#include "utils/StringUtils.h"
#include "db/DBRecord.h"
#include "commands/FireCommand.h"
#include "commands/MineCommand.h"
#include "commands/MoveCommand.h"
#include "commands/PingCommand.h"
#include "commands/SleepCommand.h"
#include "commands/SprintCommand.h"
#include "commands/SurfaceCommand.h"

namespace subsim
{

//-----------------------------------------------------------------------------
bool
Game::addCommand(const int handle, Input& input, std::string& err) {
  if (handle < 1) {
    throw Error(Msg() << "Game.addCommand() Invalid player handle: " << handle);
  } else if (!players.count(handle)) {
    throw Error(Msg() << "Game.adCommand() Unknown player handle: " << handle);
  }

  std::set<unsigned> subIDs;
  const unsigned playerID = static_cast<unsigned>(handle);
  for (const auto& command : commands) {
    if (command->getPlayerID() == playerID) {
      subIDs.insert(command->getSubID());
    }
  }

  if (input.getFieldCount() < 3) {
    err = "Command messages must begin with turn number and sub ID";
    return false;
  }

  const unsigned tn = input.getUInt(1, ~0U);
  if (tn != turnNumber) {
    err = ("Invalid turn number: " + input.getStr(0));
    return false;
  }

  const unsigned subID = input.getUInt(2, ~0U);
  if (subID >= config.getSubsPerPlayer()) {
    err = ("Invalid sub ID: " << toStr(subID));
    return false;
  } else if (subIDs.count(subID)) {
    err = ("Multiple commands for sub ID " + toStr(subID));
    return false;
  }

  const std::string type = input.getStr();
  if (type.size() != 1) {
    err = ("Invalid command type: " + type);
    return false;
  }

  try {
    switch (type[0]) {
    case MineCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new MineCommand(playerID, input)));
      break;
    case FireCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new FireCommand(playerID, input)));
      break;
    case MoveCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new MoveCommand(playerID, input)));
      break;
    case PingCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new PingCommand(playerID, input)));
      break;
    case SprintCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SprintCommand(playerID, input)));
      break;
    case SleepCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SleepCommand(playerID, input)));
      break;
    case SurfaceCommand::TYPE:
      commands.push_back(
            std::unique_ptr<Command>(new SurfaceCommand(playerID, input)));
      break;
    default:
      err = ("Invalid command type: " + type);
      return false;
    }
  } catch (const std::exception& e) {
    err = e.what();
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------
PlayerPtr
Game::getPlayer(const int handle) const {
  if (handle > 0) {
    for (PlayerPtr player : players) {
      if (player->handle() == handle) {
        return player;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
PlayerPtr
Game::getPlayer(const std::string name) const {
  if (!isEmpty(name)) {
    for (PlayerPtr player : players) {
      if (iEqual(player->getName(), name)) {
        return player;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
std::vector<PlayerPtr>
Game::getPlayers() const {
  return std::vector<PlayerPtr>(players.begin(), players.end());
}

//-----------------------------------------------------------------------------
std::vector<PlayerPtr>
Game::playersFromAddress(const std::string address) const {
  std::vector<PlayerPtr> result;
  if (!isEmpty(address)) {
    for (PlayerPtr player : players) {
      if (player->getAddress() == address) {
        result.push_back(player);
      }
    }
  }
  return std::move(result);
}

//-----------------------------------------------------------------------------
void
Game::abort() noexcept {
  if (!aborted) {
    aborted = Timer::now();
  }
}

//-----------------------------------------------------------------------------
void
Game::finish() noexcept {
  if (!finished) {
    finished = Timer::now();
  }
}

//-----------------------------------------------------------------------------
void
Game::start() {
  config.validate();
  if (started) {
    throw Error("Game is already started");
  } else if (!canStart()) {
    throw Error("Game cannot start");
  } else if (turnNumber) {
    throw Error("Game turn number is " << turnNumber << " at game start!");
  }
  started = Timer::now();
  turnNumber = 1;
}

//-----------------------------------------------------------------------------
void
Game::nextTurn() {
  if (!started) {
    throw Error("Game.nextTurn() game has not been started");
  } else if (!turnNumber) {
    throw Error("Game.nextTurn() turn number has not been initialized!");
  } else if (history.size() != (turnNumber - 1)) {
    throw Error(Msg() << "Game.nextTurn() history size " << history.size()
                << " != " << (turnNumber - 1));
  }

//  history.push_back(decltype(commands));
//  decltype(commands)& commandList = history.back();
//  for (auto it = commands.begin(); it != commands.end(); ++it) {
//    commandList.push_back(std::move(*it));
//  }

  history.push_back(std::move(commands));
  commands.clear();

  if (turnNumber >= config.getMaxTurns()) {
    finish();
  } else if (players.size() < 2) {
    Logger::error() << "Aborting game because player count dropped to "
                    << players.size();
    abort();
  }
  else {
    turnNumber++;
  }
}

//-----------------------------------------------------------------------------
void
Game::addPlayer(PlayerPtr player) {
  if (!player) {
    throw Error("Game::addPlayer() null player");
  } else if (started) {
    throw Error("Game::addPlayer() game has already started");
  } else if (isEmpty(player->getName())) {
    throw Error("game::addPlayer() empty player name");
  } else if (player->handle() <= 0) {
    throw Error(Msg() << "Game::addPlayer() invalid player handle: "
                << player->handle());
  } else if (getPlayer(player->getName())) {
    throw Error(Msg() << "Game::addPlayer() duplicate player name: "
                << player->getName());
  }

  players.push_back(player);
}

//-----------------------------------------------------------------------------
void
Game::removePlayer(const int handle) {
  for (auto it = commands.begin(); it != commands.end(); ++it) {
    if (it->getPlayerID() == handle) {
      it = commands.erase(it);
    } else {
      it++;
    }
  }
  for (auto it = players.begin(); it != players.end(); ++it) {
    PlayerPtr& player = (*it);
    if (player->handle() == handle) {
      players.erase(it);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void
Game::saveResults(Database& db) const {
  if (isEmpty(title)) {
    throw Error(Msg() << "Can't same results of a game with no title");
  }

  config.validate();

  unsigned hits = 0;
  unsigned highScore = 0;
  unsigned lowScore = ~0U;
  for (auto& player : players) {
    hits += player->getScore();
    highScore = std::max<unsigned>(highScore, player->getScore());
    lowScore = std::min<unsigned>(lowScore, player->getScore());
  }

  unsigned ties = 0;
  for (auto& player : players) {
    ties += (player->getScore() == highScore);
  }
  if (ties > 0) {
    ties--;
  } else {
    Logger::error() << "Error calculating ties for game '" << getTitle() << "'";
  }

  std::shared_ptr<DBRecord> stats = db.get(("game." + getTitle()), true);
  if (!stats) {
    throw Error(Msg() << "Failed to get stats record for game title '"
                << getTitle() << "' from '" << db << "'");
  }

  config.saveTo(*stats);

  Milliseconds count = stats->incUInt("gameCount");
  Milliseconds elapsed = elapsedTime();
  Milliseconds totalMS = (stats->getUInt64("total.timeMS") + elapsed);
  Milliseconds avgMS = count ? ((totalMS + count - 1) / count) : 0;

  stats->setString("averageTime", Timer(Timer::now() - avgMS).toString());
  stats->setString("total.time", Timer(Timer::now() - totalMS).toString());
  stats->incUInt64("total.timeMS", elapsed);
  stats->incUInt("total.aborted", (aborted ? 1 : 0));
  stats->incUInt("total.turnNumber", turnNumber);
  stats->incUInt("total.playerCount", players.size());
  stats->incUInt("total.hits", hits);
  stats->incUInt("total.ties", ties);

  stats->setString("last.time", Timer(Timer::now() - elapsed).toString());
  stats->setUInt64("last.timeMS", elapsed);
  stats->setBool("last.aborted", aborted);
  stats->setUInt("last.turnNumber", turnNumber);
  stats->setUInt("last.playerCount", players.size());
  stats->setUInt("last.hits", hits);
  stats->setUInt("last.ties", ties);

  for (auto& player : players) {
    const bool first = (player->getScore() == highScore);
    const bool last = (player->getScore() == lowScore);
    player->addStatsTo(*stats, first, last);

    std::string recordID = ("player." + player->getName());
    auto record = db.get(recordID, true);
    if (!record) {
      throw Error(Msg() << "Failed to get record for player '"
                  << player->getName() << "' from '" << db << "'");
    }
    player->saveTo((*record), (players.size() - 1), first, last);
  }
}

//-----------------------------------------------------------------------------
bool
Game::canStart() const noexcept {
  if (isEmpty(title)) {
    return false;
  }
  if (players.size() < config.getMinPlayers()) {
    return false;
  }
  if (config.getMaxPlayers() && (players.size() > config.getMaxPlayers())) {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::allCommandsReceived() const noexcept {
  return (commands.size() == (players.size() * config.getSubsPerPlayer()));
}

//-----------------------------------------------------------------------------
unsigned
Game::getMaxRange() const {
  // TODO find max sonar/torpedo range of all active submarines
  return std::max<unsigned>(config.getMapWidth(), config.getMapHeight());
}

//-----------------------------------------------------------------------------
std::map<int, std::string>
Game::executeTurn() {
  errs.clear();

  gameMap.updateDistances(getMaxRange());

  executeSleeps();
  executeMoves();
  executeSprints();
  executeMineDeployments();
  executeFireTorpedos();
  executeNuclearDetonations();
  executeSurfaces();
  executePings();

  return errs;
}

} // namespace subsim
