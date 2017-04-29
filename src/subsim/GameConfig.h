//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_CONFIGURATION_H
#define SUBSIM_CONFIGURATION_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Version.h"
#include "GameSetting.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameConfig {
//-----------------------------------------------------------------------------
public: // constants
  static const unsigned MIN_TURN_COUNT      = 100;
  static const unsigned MIN_MAP_WIDTH       = 10;
  static const unsigned MIN_MAP_HEIGHT      = 10;
  static const unsigned MAX_MAP_WIDTH       = 200;
  static const unsigned MAX_MAP_HEIGHT      = 200;
  static const unsigned MAX_SUBS_PER_PLAYER = 20;

//-----------------------------------------------------------------------------
private: // variables
  unsigned minPlayers = 2;
  unsigned maxPlayers = 2;
  unsigned maxTurns = 0;
  unsigned mapWidth = 20;
  unsigned mapHeight = 20;
  unsigned subsPerPlayer = 1;
  std::vector<Coordinate> obstacles;
  std::vector<UniqueSubPtr> submarineConfigs = {std::make_unique<Submarine>(0)};
  std::vector<GameSetting> customSettings;

//-----------------------------------------------------------------------------
public: // static methods
  static GameConfig getDefaultGameConfig();

//-----------------------------------------------------------------------------
public: // methods
  GameConfig& addCustomSetting(const GameSetting& setting);
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

  const std::vector<UniqueSubPtr>& getSubmarineConfigs() const noexcept {
    return submarineConfigs;
  }

  const std::vector<GameSetting>& getCustomSettings() const noexcept {
    return customSettings;
  }
};

} // namespace subsim

#endif // SUBSIM_CONFIGURATION_H
