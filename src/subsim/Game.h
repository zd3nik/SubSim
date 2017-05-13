//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_H
#define SUBSIM_GAME_H

#include "utils/Platform.h"
#include "utils/Input.h"
#include "utils/Timer.h"
#include "db/Database.h"
#include "commands/Command.h"
#include "commands/FireCommand.h"
#include "commands/MineCommand.h"
#include "commands/MoveCommand.h"
#include "commands/PingCommand.h"
#include "commands/SleepCommand.h"
#include "commands/SprintCommand.h"
#include "commands/SurfaceCommand.h"
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
  std::list<SubmarinePtr> nuclearDetonations;
  std::list<std::pair<Coordinate, unsigned>> detonations;
  std::map<unsigned, std::list<std::pair<Coordinate, unsigned>>> discovered;
  std::map<unsigned, std::map<Coordinate, unsigned>> torpedoHits;
  std::map<unsigned, std::map<Coordinate, unsigned>> mineHits;
  std::map<unsigned, unsigned> points;
  std::map<unsigned, std::string> errs;
  Timestamp started = 0;
  Timestamp aborted = 0;
  Timestamp finished = 0;
  unsigned turnNumber = 0;
  unsigned maxRange = 0;
  unsigned sonarActivations = 0;
  unsigned sprintActivations = 0;

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

  std::map<unsigned, std::string> executeTurn();

  void reset(const GameConfig& gameConfig, const std::string& gameTitle);
  void printSummary(Coordinate&) const;
  void abort() noexcept;
  void finish() noexcept;
  void start();

  std::string addPlayer(PlayerPtr, Input&);
  void removePlayer(const int playerHandle);
  void saveResults(Database&) const;

//-----------------------------------------------------------------------------
private: // methods
  void sendToAll(const std::string& message);
  bool sendTo(Player&, const std::string& message);
  bool sendDiscoveredObjects(Player&);
  bool sendTorpedoHits(Player&);
  bool sendMineHits(Player&);
  bool sendSubInfo(Player&);
  bool sendScore(Player&);

  void exec(const Command::CommandType);
  void exec(SubmarinePtr&, const SleepCommand&);
  void exec(SubmarinePtr&, const MoveCommand&);
  void exec(SubmarinePtr&, const SprintCommand&);
  void exec(SubmarinePtr&, const MineCommand&);
  void exec(SubmarinePtr&, const FireCommand&);
  void exec(SubmarinePtr&, const SurfaceCommand&);
  void exec(SubmarinePtr&, const PingCommand&);

  void executeNuclearDetonations();
  void executeRepairs();
  bool detonateMines(Square&);
  void detonationFrom(Player&, const unsigned type, Square&);
  void inflictDamageFrom(Player&, const unsigned type, Square&,
                         const unsigned damage);

  unsigned blastDistance(const Coordinate& from, const Coordinate& to) const;
  std::vector<Coordinate> getBlastCoordinates(const Coordinate&,
                                              const unsigned range) const;
};

} // namespace subsim

#endif // SUBSIM_GAME_H
