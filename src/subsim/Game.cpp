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

namespace subsim
{

//-----------------------------------------------------------------------------
void
Game::clear() noexcept {
  started = 0;
  aborted = 0;
  finished = 0;
  turnNumber = 0;
  config.clear();
  players.clear();
  // TODO
}

//-----------------------------------------------------------------------------
void
Game::addPlayer(const SubmarinePtr& player) {
  if (!player) {
    throw Error("Game.addPlayer() null player");
  }
  if (isEmpty(player->getName())) {
    throw Error("Game.addPlayer() empty name");
  }
  if (hasPlayer(player->getName())) {
    throw Error(Msg() << "Game.addPlayer() duplicate name: "
                << player->getName());
  }
  if (hasPlayer(player->handle())) {
    throw Error(Msg() << "Game.addPlayer() duplicate handle: "
                << player->handle());
  }
  players.push_back(player);
}

//-----------------------------------------------------------------------------
SubmarinePtr
Game::playerAtIndex(const unsigned index) const {
  if (index < players.size()) {
    return players[index];
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
SubmarinePtr
Game::playerForHandle(const int handle) const {
  if (handle >= 0) {
    for (auto& player : players) {
      if (player->handle() == handle) {
        return player;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
SubmarinePtr
Game::playerForName(const std::string& name, const bool exact) const {
  if (name.empty()) {
    return nullptr;
  }

  if (!exact && isUInt(name)) {
    const unsigned idx = toUInt32(name);
    if (idx) {
      return playerAtIndex(idx - 1);
    }
  }

  SubmarinePtr match;
  for (auto& player : players) {
    const std::string playerName = player->getName();
    if (playerName == name) {
      return player;
    } else if (!exact && iEqual(playerName, name, name.size())) {
      if (match) {
        return nullptr;
      } else {
        match = player;
      }
    }
  }
  return match;
}

//-----------------------------------------------------------------------------
bool
Game::start(const bool randomize) {
  if (!isValid()) {
    Logger::error() << "can't start game because it is not valid";
    return false;
  }
  if (isStarted()) {
    Logger::error() << "can't start game because it is already started";
    return false;
  }
  if (isFinished()) {
    Logger::error() << "can't start game because it is has finished";
    return false;
  }

  Logger::debug() << "starting game '" << getTitle() << "'";

  if (randomize) {
    std::random_shuffle(players.begin(), players.end());
  }

  started = Timer::now();
  aborted = 0;
  finished = 0;
  toMove = 0;
  turnCount = 0;

  updatePlayerToMove();
  return true;
}

//-----------------------------------------------------------------------------
bool
Game::nextTurn() {
  if (!isStarted()) {
    throw Error("Game.nextTurn() game has not been started");
  }
  if (isFinished()) {
    throw Error("Game.nextTurn() game has finished");
  }

  turnCount += !toMove;
  if (++toMove >= players.size()) {
    toMove = 0;
  }

  updatePlayerToMove();

  unsigned minTurns = ~0U;
  unsigned maxTurns = 0;
  unsigned maxScore = 0;
  unsigned dead = 0;
  for (auto& player : players) {
    minTurns = std::min<unsigned>(minTurns, player->getTurns());
    maxTurns = std::max<unsigned>(maxTurns, player->getTurns());
    maxScore = std::max<unsigned>(maxScore, player->getScore());
    if (player->isDead()) {
      dead++;
    }
  }
  if ((dead >= players.size()) ||
      ((maxScore >= config.getPointGoal()) && (minTurns >= maxTurns)))
  {
    finished = Timer::now();
  }

  return !isFinished();
}

//-----------------------------------------------------------------------------
void
Game::disconnectPlayer(const std::string& name, const std::string& msg) {
  if (!isStarted()) {
    throw Error("Game.disconnectPlayer() game has not started");
  }
  auto player = playerForPlayer(name, true);
  if (player) {
    player->setStatus(msg.size() ? msg : "disconnected");
    player->disconnect();
  }
}

//-----------------------------------------------------------------------------
void
Game::removePlayer(const std::string& name) {
  if (isStarted()) {
    throw Error("Game.removePlayer() game has started");
  }
  for (auto it = players.begin(); it != players.end(); ++it) {
    if ((*it)->getName() == name) {
      players.erase(it);
      break;
    }
  }
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
Game::saveResults(Database& db) {
  if (!isValid()) {
    throw Error("Cannot save invalid game");
  }

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
  stats->incUInt("total.turnCount", turnCount);
  stats->incUInt("total.playerCount", players.size());
  stats->incUInt("total.hits", hits);
  stats->incUInt("total.ties", ties);

  stats->setString("last.time", Timer(Timer::now() - elapsed).toString());
  stats->setUInt64("last.timeMS", elapsed);
  stats->setBool("last.aborted", aborted);
  stats->setUInt("last.turnCount", turnCount);
  stats->setUInt("last.playerCount", players.size());
  stats->setUInt("last.hits", hits);
  stats->setUInt("last.ties", ties);

  for (auto& player : players) {
    const bool first = (player->getScore() == highScore);
    const bool last = (player->getScore() == lowScore);
    player->addStatsTo(*stats, first, last);

    std::string recordID = ("player." + player->getName());
    std::shared_ptr<DBRecord> player = db.get(recordID, true);
    if (!player) {
      throw Error(Msg() << "Failed to get record for player '"
                  << player->getName() << "' from '" << db << "'");
    }
    player->saveTo(*player, (players.size() - 1), first, last);
  }
}

//-----------------------------------------------------------------------------
bool
Game::isValid() const noexcept {
  return (config &&
          (players.size() >= config.getMinPlayers()) &&
          (players.size() <= config.getMaxPlayers()));
}

} // namespace subsim
