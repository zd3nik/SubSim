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
Game::deployMine(const int handle) {
  throw Error("TODO Game::deployMine()");
}

//-----------------------------------------------------------------------------
void
Game::fireTorpedo(const int handle) {
  throw Error("TODO Game::fireTorpedo()");
}

//-----------------------------------------------------------------------------
void
Game::moveSubmarine(const int handle) {
  throw Error("TODO Game::moveSubmarine()");
}

//-----------------------------------------------------------------------------
void
Game::sleep(const int handle) {
  throw Error("TODO Game::seep()");
}

//-----------------------------------------------------------------------------
void
Game::sonarPing(const int handle) {
  throw Error("TODO Game::sonarPing()");
}

//-----------------------------------------------------------------------------
void
Game::sprint(const int handle) {
  throw Error("TODO Game::sprint()");
}

//-----------------------------------------------------------------------------
void
Game::surface(const int handle) {
  throw Error("TODO Game::surface");
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
  throw Error("TODO Game::start()");
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
Game::isValid() const noexcept {
  if (isEmpty(title)) {
    return false;
  }

  try {
    config.validate();
  } catch (...) {
    return false;
  }

  return true;
}

} // namespace subsim
