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
#include "Player.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Game {
//-----------------------------------------------------------------------------
private: // variables
  std::string title;
  GameConfig config;
  GameMap gameMap;
  std::vector<PlayerPtr> players;
  Timestamp started = 0;
  Timestamp aborted = 0;
  Timestamp finished = 0;
  unsigned turnNumber = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Game() = default;
  Game(Game&&) = delete;
  Game(const Game&) = delete;
  Game& operator=(Game&&) = delete;
  Game& operator=(const Game&) = delete;

//-----------------------------------------------------------------------------
public: // inline methods
  const std::string& getTitle() const noexcept { return title; }
  const GameConfig& getConfig() const noexcept { return config; }
  const GameMap& getMap() const noexcept { return gameMap; }

  bool isAborted() const noexcept { return aborted; }
  bool isFinished() const noexcept { return (aborted || finished); }
  bool isStarted() const noexcept { return started; }

  unsigned getPlayerCount() const noexcept { return players.size(); }
  unsigned getTurnNumber() const noexcept { return turnNumber; }

  void clearPlayers() { players.clear(); }
  void reset(const GameConfig& gameConfig, const std::string& gameTitle) {
    title = gameTitle;
    config = gameConfig;
    gameMap.reset(config.getMapWidth(), config.getMapHeight());
    started = 0;
    aborted = 0;
    finished = 0;
    turnNumber = 0;
  }

  Milliseconds elapsedTime() const noexcept {
    return finished ? (finished - started) : aborted ? (aborted - started) : 0;
  }

//-----------------------------------------------------------------------------
public: // submarine command methods
  void deployMine(const int playerHandle);
  void fireTorpedo(const int playerHandle);
  void moveSubmarine(const int playerHandle);
  void sleep(const int playerHandle);
  void sonarPing(const int playerHandle);
  void sprint(const int playerHandle);
  void surface(const int playerHandle);

//-----------------------------------------------------------------------------
public: // other methods
  PlayerPtr getPlayer(const int playerHandle) const;
  PlayerPtr getPlayer(const std::string playerName,
                      const bool exactMatch = true) const;

  std::vector<PlayerPtr> getPlayers() const;
  std::vector<PlayerPtr> playersForAddress(const std::string address) const;

  void abort() noexcept;
  void finish() noexcept;
  void start();

  void addPlayer(PlayerPtr);
  void removePlayer(const int playerHandle);
  void saveResults(Database&) const;

//-----------------------------------------------------------------------------
public: // operator overloads
  bool isValid() const noexcept;
};

} // namespace subsim

#endif // SUBSIM_GAME_H
