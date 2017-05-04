//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_CONFIGURATION_H
#define SUBSIM_CONFIGURATION_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Version.h"
#include "db/DBRecord.h"
#include "GameSetting.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameConfig {
//-----------------------------------------------------------------------------
public: // constants
  static const unsigned MIN_TURN_COUNT      = 100;
  static const unsigned MIN_SUB_SIZE        = 50;
  static const unsigned MAX_SUB_SIZE        = 200;
  static const unsigned MIN_TURN_TIMEOUT    = 500;
  static const unsigned MIN_MAP_WIDTH       = 10;
  static const unsigned MIN_MAP_HEIGHT      = 10;
  static const unsigned MAX_MAP_WIDTH       = 200;
  static const unsigned MAX_MAP_HEIGHT      = 200;
  static const unsigned MAX_SUBS_PER_PLAYER = 20;

  static const unsigned DEFAULT_MIN_PLAYERS     = 2;
  static const unsigned DEFAULT_MAX_PLAYERS     = 0;
  static const unsigned DEFAULT_MAX_TURNS       = 0;
  static const unsigned DEFAULT_TURN_TIMEOUT    = 0;
  static const unsigned DEFAULT_MAP_WIDTH       = 20;
  static const unsigned DEFAULT_MAP_HEIGHT      = 20;
  static const unsigned DEFAULT_SUBS_PER_PLAYER = 1;

//-----------------------------------------------------------------------------
private: // variables
  unsigned minPlayers    = DEFAULT_MIN_PLAYERS;
  unsigned maxPlayers    = DEFAULT_MAX_PLAYERS;
  unsigned maxTurns      = DEFAULT_MAX_TURNS;
  unsigned turnTimeout   = DEFAULT_TURN_TIMEOUT;
  unsigned mapWidth      = DEFAULT_MAP_WIDTH;
  unsigned mapHeight     = DEFAULT_MAP_HEIGHT;
  unsigned subsPerPlayer = DEFAULT_SUBS_PER_PLAYER;
  std::vector<Coordinate> obstacles;
  std::vector<GameSetting> customSettings;
  std::vector<Submarine> submarineConfigs;

//-----------------------------------------------------------------------------
public: // constructors
  GameConfig();
  GameConfig(GameConfig&&) = default;
  GameConfig(const GameConfig&) = default;
  GameConfig& operator=(GameConfig&&) = default;
  GameConfig& operator=(const GameConfig&) = default;

//-----------------------------------------------------------------------------
public: // methods
  void print(const std::string& title, Coordinate&) const;
  void addSetting(const GameSetting& setting);
  void loadFrom(const DBRecord&);
  void saveTo(DBRecord&) const;
  void validate() const;

//-----------------------------------------------------------------------------
public: // getters
  std::string toMessage(const Version&, const std::string& title) const;
  unsigned getMinPlayers() const noexcept { return minPlayers; }
  unsigned getMaxPlayers() const noexcept { return maxPlayers; }
  unsigned getMaxTurns() const noexcept { return maxTurns; }
  unsigned getMapWidth() const noexcept { return mapWidth; }
  unsigned getMapHeight() const noexcept { return mapHeight; }
  unsigned getSubsPerPlayer() const noexcept { return subsPerPlayer; }

  const std::vector<Coordinate>& getObstacles() const noexcept {
    return obstacles;
  }

  const std::vector<GameSetting>& getCustomSettings() const noexcept {
    return customSettings;
  }

  const std::vector<Submarine>& getSubmarineConfigs() const noexcept {
    return submarineConfigs;
  }

private: // methods
  void get(const GameSetting::SettingType type, const DBRecord&);
  void store(const GameSetting::SettingType type, DBRecord&) const;
};

} // namespace subsim

#endif // SUBSIM_CONFIGURATION_H
