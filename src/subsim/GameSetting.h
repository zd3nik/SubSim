//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_SETTING_H
#define SUBSIM_GAME_SETTING_H

#include "utils/Platform.h"
#include "utils/StringUtils.h"

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
    TurnTimeout,
    MapSize,
    Obstacle,
    SubsPerPlayer,
    SubSize,
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
  SettingType type = None;
  std::vector<std::string> values;

//-----------------------------------------------------------------------------
public: // constructos
  GameSetting() = default;
  GameSetting(GameSetting&&) = default;
  GameSetting(const GameSetting&) = default;
  GameSetting& operator=(GameSetting&&) = default;
  GameSetting& operator=(const GameSetting&) = default;

  GameSetting(const SettingType type)
    : type(type)
  { }

  GameSetting(const SettingType type, const std::string& stringValue)
    : type(type)
  {
    values.push_back(stringValue);
  }

  GameSetting(const SettingType type, const unsigned unsignedValue)
    : type(type)
  {
    values.push_back(toStr(unsignedValue));
  }

  GameSetting(const SettingType type,
              const std::vector<std::string>& stringValues)
    : type(type),
      values(stringValues.begin(), stringValues.end())
  { }

  GameSetting(const SettingType type,
              const std::vector<unsigned>& unsignedValues)
    : type(type)
  {
    values.reserve(unsignedValues.size());
    for (unsigned value : unsignedValues) {
      values.push_back(toStr(value));
    }
  }

//-----------------------------------------------------------------------------
public: // static methods
  static std::string typeName(const SettingType);
  static SettingType getType(const std::string& name);
  static GameSetting fromMessage(const std::string& message);

//-----------------------------------------------------------------------------
public: // setters
  void clearValues() {
    values.clear();
  }

  void addValue(const std::string& value) {
    values.push_back(value);
  }

  template<typename T>
  void addValue(const T& value) {
    values.push_back(toStr(value));
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
