//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_SETTING_H
#define SUBSIM_GAME_SETTING_H

#include "utils/Platform.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameSetting {
//-----------------------------------------------------------------------------
public: // enums
  enum SettingType {
    None,
    MinPlayers,
    MaxPlayers,
    MaxTurns,
    MapSize,
    Obstacle,
    SubsPerPlayer,
    SubStartLocation,
    SubSurfaceTurnCount,
    SubMaxShields,
    SubMaxReactorDamage,
    SubMaxSonarCharge,
    SubMaxTorpedoCharge,
    SubMaxMineCharge,
    SubMaxSprintCharge,
    SubTorpedoCount,
    SubMineCount
  };

//-----------------------------------------------------------------------------
public: // variables
  const SettingType type = None;
  std::vector<std::string> values;

//-----------------------------------------------------------------------------
public: // constructos
  GameSetting(const SettingType type)
    : type(type)
  { }

//-----------------------------------------------------------------------------
public: // static methods
  static std::string typeName(const SettingType);
  static SettingType getType(const std::string& name);
  static GameSetting fromMessage(const std::string& message);

//-----------------------------------------------------------------------------
public: // setters
  GameSetting& clearValues() {
    values.clear();
    return (*this);
  }

  GameSetting& addValue(const std::string& value) {
    values.push_back(value);
    return (*this);
  }

  template<typename T>
  GameSetting& addValue(const T& value) {
    values.push_back(toStr(value));
    return (*this);
  }

//-----------------------------------------------------------------------------
public: // getters
  std::string getName() const {
    return typeName(type);
  }

  SettingType getType() const noexcept {
    return type;
  }

  unsigned getValueCount() const noexcept {
    return values.size();
  }

  std::string getStrValue(const unsigned idx = 0) const {
    return values[idx];
  }

  const std::vector<std::string>& getStrValues() const noexcept {
    return values;
  }

  unsigned getUnsignedValue(const unsigned idx = 0,
                            const unsigned def = 0) const;

  std::vector<unsigned> getUnsignedValues(const unsigned def = 0) const;
  std::string toMessage() const;
};

} // namespace subsim

#endif // SUBSIM_GAME_SETTING_H
