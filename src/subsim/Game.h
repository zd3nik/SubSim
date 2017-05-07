//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_H
#define SUBSIM_GAME_H

#include "utils/Platform.h"
#include "utils/Input.h"
#include "utils/Timer.h"
#include "db/Database.h"
#include "Command.h"
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
  std::map<int, PlayerPtr> players;
  std::list<UniqueCommand> commands;
  std::vector<std::list<UniqueCommand>> history;
  std::map<int, std::string> errs;
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
public: // methods
  bool canStart() const noexcept;
  bool allCommandsReceived() const noexcept;
  bool addCommand(const int playerHandle, Input&, std::string& err);

  PlayerPtr getPlayer(const int playerHandle) const;
  PlayerPtr getPlayer(const std::string playerName) const;

  std::vector<PlayerPtr> getPlayers() const;
  std::vector<PlayerPtr> playersFromAddress(const std::string address) const;

  std::map<int, std::string> executeTurn();

  void printSummary(Coordinate&) const;
  void abort() noexcept;
  void finish() noexcept;
  void start();
  void nextTurn();

  std::string addPlayer(PlayerPtr, Input&);
  void removePlayer(const int playerHandle);
  void saveResults(Database&) const;

//-----------------------------------------------------------------------------
private: // methods
  unsigned getMaxRange() const;
  void executeSleeps();
  void executeMoves();
  void executeSprints();
  void executeMineDeployments();
  void executeFireTorpedos();
  void executeNuclearDetonations();
  void executeSurfaces();
  void executePings();
};

} // namespace subsim

#endif // SUBSIM_GAME_H
