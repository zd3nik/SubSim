//-----------------------------------------------------------------------------
// GameConfig.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_CONFIGURATION_H
#define SUBSIM_CONFIGURATION_H

#include "Platform.h"
#include "Coordinate.h"
#include "Input.h"
#include "SubmarineBase.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameConfig {
//-----------------------------------------------------------------------------
private: // variables
  std::string title = "default";
  unsigned minPlayers = 2;
  unsigned maxPlayers = 2;
  unsigned maxTurns = 0;
  unsigned width = 20;
  unsigned height = 20;
  unsigned mineCharge = 3;
  std::vector<Coordinate> obstacles;
  std::vector<Coordinate> presetLocations;
  std::vector<SubmarineBase> submarineConfigs = { SubmarineBase() };

//-----------------------------------------------------------------------------
public: // static methods
  static GameConfig getDefaultGameConfig();

//-----------------------------------------------------------------------------
public: // methods
  std::string getTitle() const { return title; }
  unsigned getMinPlayers() const noexcept { return minPlayers; }
  unsigned getMaxPlayers() const noexcept { return maxPlayers; }
  unsigned getMaxTurns() const noexcept { return maxTurns; }
  unsigned getWidth() const noexcept { return width; }
  unsigned getHeight() const noexcept { return height; }
  unsigned getMineCharge() const noexcept { return mineCharge; }

  const std::vector<Coordinate> getObstacles() const noexcept {
    return obstacles;
  }

  const std::vector<Coordinate> getPresetLocations() const noexcept {
    return presetLocations;
  }

  const std::vector<SubmarineBase> getSubmarineConfigs() const noexcept {
    return submarineConfigs;
  }
};

} // namespace subsim

#endif // SUBSIM_CONFIGURATION_H
