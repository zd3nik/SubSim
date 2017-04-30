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
Game& Game::clear() noexcept {
  started = 0;
  aborted = 0;
  finished = 0;
  toMove = 0;
  turnCount = 0;
  config.clear();
  boards.clear();
  return (*this);
}

//-----------------------------------------------------------------------------
Game& Game::addBoard(const BoardPtr& board) {
  if (!board) {
    throw Error("Game.addBoard() null board");
  }
  if (isEmpty(board->getName())) {
    throw Error("Game.addBoard() empty name");
  }
  if (hasBoard(board->getName())) {
    throw Error(Msg() << "Game.addBoard() duplicate name: "
                << board->getName());
  }
  if (hasBoard(board->handle())) {
    throw Error(Msg() << "Game.addBoard() duplicate handle: "
                << board->handle());
  }
  boards.push_back(board);
  return (*this);
}

//-----------------------------------------------------------------------------
Game& Game::setConfiguration(const Configuration& value) {
  config = value;
  return (*this);
}

//-----------------------------------------------------------------------------
Game& Game::setTitle(const std::string& value) {
  config.setName(value);
  return (*this);
}

//-----------------------------------------------------------------------------
BoardPtr Game::boardToMove() const {
  if (isStarted() && !isFinished()) {
    auto board = boardAtIndex(toMove);
    if (!board || !board->isToMove()) {
      throw Error("Game.getBoardToMove() board.toMove is not in sync!");
    }
    return board;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
BoardPtr Game::boardAtIndex(const unsigned index) const {
  if (index < boards.size()) {
    return boards[index];
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
BoardPtr Game::boardForHandle(const int handle) const {
  if (handle >= 0) {
    for (auto& board : boards) {
      if (board->handle() == handle) {
        return board;
      }
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
BoardPtr Game::boardForPlayer(const std::string& name, const bool exact) const {
  if (name.empty()) {
    return nullptr;
  }

  if (!exact && isUInt(name)) {
    const unsigned idx = toUInt32(name);
    if (idx) {
      return boardAtIndex(idx - 1);
    }
  }

  BoardPtr match;
  for (auto& board : boards) {
    const std::string playerName = board->getName();
    if (playerName == name) {
      return board;
    } else if (!exact && iEqual(playerName, name, name.size())) {
      if (match) {
        return nullptr;
      } else {
        match = board;
      }
    }
  }
  return match;
}

//-----------------------------------------------------------------------------
bool Game::hasOpenBoard() const {
  if (!isFinished()) {
    if (isStarted()) {
      for (auto& board : boards) {
        if (!board->isConnected()) {
          return true;
        }
      }
    } else {
      return (boards.size() < config.getMaxPlayers());
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool Game::start(const bool randomize) {
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
    std::random_shuffle(boards.begin(), boards.end());
  }

  started = Timer::now();
  aborted = 0;
  finished = 0;
  toMove = 0;
  turnCount = 0;

  updateBoardToMove();
  return true;
}

//-----------------------------------------------------------------------------
bool Game::nextTurn() {
  if (!isStarted()) {
    throw Error("Game.nextTurn() game has not been started");
  }
  if (isFinished()) {
    throw Error("Game.nextTurn() game has finished");
  }

  turnCount += !toMove;
  if (++toMove >= boards.size()) {
    toMove = 0;
  }

  updateBoardToMove();

  unsigned minTurns = ~0U;
  unsigned maxTurns = 0;
  unsigned maxScore = 0;
  unsigned dead = 0;
  for (auto& board : boards) {
    minTurns = std::min<unsigned>(minTurns, board->getTurns());
    maxTurns = std::max<unsigned>(maxTurns, board->getTurns());
    maxScore = std::max<unsigned>(maxScore, board->getScore());
    if (board->isDead()) {
      dead++;
    }
  }
  if ((dead >= boards.size()) ||
      ((maxScore >= config.getPointGoal()) && (minTurns >= maxTurns)))
  {
    finished = Timer::now();
  }

  return !isFinished();
}

//-----------------------------------------------------------------------------
bool Game::setNextTurn(const std::string& name) {
  if (name.empty()) {
    throw Error("Game.nextTurn() empty board name");
  }
  if (!isStarted()) {
    throw Error(Msg() << "Game.nextTurn(" << name << ") game is not started");
  }
  if (isFinished()) {
    throw Error(Msg() << "Game.nextTurn(" << name << ") game is finished");
  }

  unsigned idx = ~0U;
  for (unsigned i = 0; i < boards.size(); ++i) {
    if (boards[i]->getName() == name) {
      idx = i;
      break;
    }
  }

  if (idx == ~0U) {
    return false;
  }

  toMove = idx;
  updateBoardToMove();
  return true;
}

//-----------------------------------------------------------------------------
void Game::disconnectBoard(const std::string& name, const std::string& msg) {
  if (!isStarted()) {
    throw Error("Game.disconnectBoard() game has not started");
  }
  auto board = boardForPlayer(name, true);
  if (board) {
    board->setStatus(msg.size() ? msg : "disconnected");
    board->disconnect();
  }
}

//-----------------------------------------------------------------------------
void Game::removeBoard(const std::string& name) {
  if (isStarted()) {
    throw Error("Game.removeBoard() game has started");
  }
  for (auto it = boards.begin(); it != boards.end(); ++it) {
    if ((*it)->getName() == name) {
      boards.erase(it);
      break;
    }
  }
}

//-----------------------------------------------------------------------------
void Game::abort() noexcept {
  if (!aborted) {
    aborted = Timer::now();
  }
}

//-----------------------------------------------------------------------------
void Game::finish() noexcept {
  if (!finished) {
    finished = Timer::now();
  }
}

//-----------------------------------------------------------------------------
void Game::saveResults(Database& db) {
  if (!isValid()) {
    throw Error("Cannot save invalid game");
  }

  unsigned hits = 0;
  unsigned highScore = 0;
  unsigned lowScore = ~0U;
  for (auto& board : boards) {
    hits += board->getScore();
    highScore = std::max<unsigned>(highScore, board->getScore());
    lowScore = std::min<unsigned>(lowScore, board->getScore());
  }

  unsigned ties = 0;
  for (auto& board : boards) {
    ties += (board->getScore() == highScore);
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
  stats->incUInt("total.playerCount", boards.size());
  stats->incUInt("total.hits", hits);
  stats->incUInt("total.ties", ties);

  stats->setString("last.time", Timer(Timer::now() - elapsed).toString());
  stats->setUInt64("last.timeMS", elapsed);
  stats->setBool("last.aborted", aborted);
  stats->setUInt("last.turnCount", turnCount);
  stats->setUInt("last.playerCount", boards.size());
  stats->setUInt("last.hits", hits);
  stats->setUInt("last.ties", ties);

  for (auto& board : boards) {
    const bool first = (board->getScore() == highScore);
    const bool last = (board->getScore() == lowScore);
    board->addStatsTo(*stats, first, last);

    std::string recordID = ("player." + board->getName());
    std::shared_ptr<DBRecord> player = db.get(recordID, true);
    if (!player) {
      throw Error(Msg() << "Failed to get record for player '"
                  << board->getName() << "' from '" << db << "'");
    }
    board->saveTo(*player, (boards.size() - 1), first, last);
  }
}

//-----------------------------------------------------------------------------
void Game::setBoardOrder(const std::vector<std::string>& order) {
  if (isStarted()) {
    throw Error("Game.setBoardOrder() game has already started");
  }

  std::vector<BoardPtr> tmp;
  tmp.reserve(boards.size());

  for (auto& name : order) {
    auto board = boardForPlayer(name, true);
    if (!board) {
      throw Error(Msg() << "Game.setBoardOrder() board name '" << name
                  << "' not found");
    }
    tmp.push_back(board);
  }

  if (tmp.size() != boards.size()) {
    throw Error(Msg() << "Game.setBoardOrder() given board list size ("
                << tmp.size() << ") doesn't match board size ("
                << boards.size() << ')');
  }

  boards.assign(tmp.begin(), tmp.end());
}

//-----------------------------------------------------------------------------
bool Game::isValid() const noexcept {
  return (config &&
          (boards.size() >= config.getMinPlayers()) &&
          (boards.size() <= config.getMaxPlayers()));
}

//-----------------------------------------------------------------------------
void Game::updateBoardToMove() noexcept {
  for (unsigned i = 0; i < boards.size(); ++i) {
    boards[i]->setToMove(i == toMove);
  }
}

} // namespace subsim
