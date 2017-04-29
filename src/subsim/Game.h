//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_H
#define SUBSIM_GAME_H

#include "utils/Platform.h"
#include "utils/Timer.h"
#include "db/Database.h"
#include "GameConfig.h"
#include "GameMap.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Game {
//-----------------------------------------------------------------------------
private: // variables
  Timestamp started = 0;
  Timestamp aborted = 0;
  Timestamp finished = 0;
  unsigned toMove = 0;
  unsigned turnCount = 0;
  Configuration config;
  std::vector<BoardPtr> boards;

//-----------------------------------------------------------------------------
public: // constructors
  Game() = default;
  Game(Game&&) = delete;
  Game(const Game&) = delete;
  Game& operator=(Game&&) = delete;
  Game& operator=(const Game&) = delete;

  explicit Game(const Configuration& config, const std::string& title = "")
    : config(config)
  {
    if (title.size()) {
      this->config.setName(title);
    }
  }

//-----------------------------------------------------------------------------
public: // methods
  Game& addBoard(const BoardPtr&);
  Game& clear() noexcept;
  Game& setConfiguration(const Configuration&);
  Game& setTitle(const std::string&);

  BoardPtr boardAtIndex(const unsigned index) const;
  BoardPtr boardForHandle(const int handle) const;
  BoardPtr boardForPlayer(const std::string& name, const bool exact) const;
  BoardPtr boardToMove() const;

  bool hasOpenBoard() const;
  bool nextTurn();
  bool setNextTurn(const std::string& name);
  bool start(const bool randomizeBoardOrder = false);

  void abort() noexcept;
  void disconnectBoard(const std::string& name, const std::string& msg);
  void finish() noexcept;
  void removeBoard(const std::string& name);
  void saveResults(Database&);
  void setBoardOrder(const std::vector<std::string>& order);

  std::string getTitle() const { return config.getName(); }
  const Configuration& getConfiguration() const noexcept { return config; }
  bool isAborted() const noexcept { return aborted; }
  bool isFinished() const noexcept { return (aborted || finished); }
  bool isStarted() const noexcept { return started; }
  unsigned getBoardCount() const noexcept { return boards.size(); }
  unsigned getTurnCount() const noexcept { return turnCount; }

  Milliseconds elapsedTime() const noexcept {
    return finished ? (finished - started) : aborted ? (aborted - started) : 0;
  }

  bool hasBoard(const std::string& name) const {
    return static_cast<bool>(boardForPlayer(name, true));
  }

  bool hasBoard(const int handle) const {
    return static_cast<bool>(boardForHandle(handle));
  }

  std::vector<BoardPtr> getBoards() const {
    return std::vector<BoardPtr>(boards.begin(), boards.end());
  }

  std::vector<BoardPtr> boardsForAddress(const std::string& address) {
    std::vector<BoardPtr> result;
    std::copy_if(boards.begin(), boards.end(), std::back_inserter(result),
      [&](const BoardPtr& b) { return (b->getAddress() == address); }
    );
    return std::move(result);
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isValid(); }

//-----------------------------------------------------------------------------
private: // methods
  bool isValid() const noexcept;
  void updateBoardToMove() noexcept;
};

} // namespace subsim

#endif // SUBSIM_GAME_H
