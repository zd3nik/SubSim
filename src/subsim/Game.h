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
#include <ostream>

namespace subsim
{

//-----------------------------------------------------------------------------
class Game {
//-----------------------------------------------------------------------------
public: // typedefs
  typedef std::list<GameMap::TorpedoShot> TorpedoShots;

//-----------------------------------------------------------------------------
private: // variables
  std::string title;
  GameConfig config;
  GameMap gameMap;
  TorpedoShots torpedoShots;
  std::map<int, PlayerPtr> players;
  std::list<UniqueCommand> commands;
  std::list<SubmarinePtr> nuclearDetonations;
  std::list<std::pair<Coordinate, unsigned>> detonations;
  std::map<unsigned, std::list<std::pair<unsigned, unsigned>>> spotted;
  std::map<unsigned, std::map<unsigned, unsigned>> sprints;
  std::map<unsigned, std::list<std::pair<Coordinate, unsigned>>> discovered;
  std::map<unsigned, std::map<Coordinate, unsigned>> torpedoHits;
  std::map<unsigned, std::map<Coordinate, unsigned>> mineHits;
  std::map<unsigned, unsigned> points;
  std::map<unsigned, std::string> errs;
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
  const TorpedoShots& shotsFired() const noexcept { return torpedoShots; }

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
  bool addCommand(const int playerHandle,
                  Input&,
                  std::string& err);

  PlayerPtr getPlayer(const int playerHandle) const;
  PlayerPtr getPlayer(const std::string playerName) const;

  std::vector<PlayerPtr> getPlayers() const;
  std::vector<PlayerPtr> playersFromAddress(const std::string address) const;

  std::map<unsigned, std::string> start(std::ostream& gameLog);
  std::map<unsigned, std::string> executeTurn(std::ostream& gameLog);

  void reset(const GameConfig& gameConfig, const std::string& gameTitle);
  void printSummary(Coordinate&) const;
  void abort() noexcept;
  void finish() noexcept;

  std::string addPlayer(PlayerPtr, Input&);
  void removePlayer(const int playerHandle);
  void saveResults(Database&) const;

//-----------------------------------------------------------------------------
private: // methods
  void sendToAll(std::ostream& gameLog, const std::string& message);
  bool sendTo(std::ostream* gameLog, Player&, const std::string& message);

  bool sendSonarDiscoveries(std::ostream& gameLog, Player&);
  bool sendSprintDetections(std::ostream& gameLog, Player&);
  bool sendDiscoveredObjects(std::ostream& gameLog, Player&);
  bool sendTorpedoHits(std::ostream& gameLog, Player&);
  bool sendMineHits(std::ostream& gameLog, Player&);
  bool sendSubInfo(std::ostream& gameLog, Player&);
  bool sendScore(std::ostream& gameLog, Player&);

  void exec(std::ostream& gameLog, const Command::CommandType);
  bool exec(SubmarinePtr&, const SleepCommand&);
  bool exec(SubmarinePtr&, const MoveCommand&);
  bool exec(SubmarinePtr&, const SprintCommand&);
  bool exec(SubmarinePtr&, const MineCommand&);
  bool exec(SubmarinePtr&, const FireCommand&);
  bool exec(SubmarinePtr&, const SurfaceCommand&);
  bool exec(SubmarinePtr&, const PingCommand&);

  void executeNuclearDetonations();
  void executeRepairs();
  bool detonateMines(Square&);
  void detonationFrom(PlayerPtr&, const Coordinate& sourceSquare,
                      const unsigned type, Square& damagedSquare);

  void inflictDamageFrom(PlayerPtr&, const Coordinate& sourceSquare,
                         const unsigned type, Square& damagedSquare,
                         const unsigned damage);

  unsigned blastDistance(const Coordinate& from, const Coordinate& to) const;

  GameMap::TorpedoShot getTorpedoShot(const std::map<Coordinate, unsigned>&,
                                      const Coordinate& from,
                                      const Coordinate& to,
                                      const unsigned blastRadius) const;

  std::vector<Coordinate> getBlastCoordinates(const Coordinate&,
                                              const unsigned range) const;
};

} // namespace subsim

#endif // SUBSIM_GAME_H
