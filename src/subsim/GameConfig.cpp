//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameConfig.h"
#include "utils/CSVWriter.h"
#include "utils/Error.h"
#include "utils/Msg.h"

namespace subsim
{

//-----------------------------------------------------------------------------
GameConfig
GameConfig::getDefaultGameConfig() {
  GameConfig config;
  for (unsigned subID = 0; subID < maxPlayers; ++subID) {
    config.submarineConfigs.push_back(std::make_unique<Submarine>(subID));
  }
  return std::move(config);
}

//-----------------------------------------------------------------------------
void
GameConfig::validate() const {
  if (minPlayers < 2) {
    throw Error(Msg() << "Invalid min player count: " << minPlayers);
  }
  if (maxPlayers < minPlayers) {
    throw Error(Msg() << "Invalid min/max player count: " << minPlayers
                << '/' << maxPlayers);
  }
  if (maxTurns < MIN_TURN_COUNT) {
    throw Error(Msg() << "Invalid max turn count: " << maxTurns);
  }
  if ((mapWidth < MIN_MAP_WIDTH) || (mapWidth > MAX_MAP_WIDTH) ||
      (mapHeight < MIN_MAP_HEIGHT) || (mapHeight > MAX_MAP_HEIGHT))
  {
    throw Error(Msg() << "Invalid map size: " << mapWidth << '|' << mapHeight);
  }
  if ((subsPerPlayer < 1) || (subsPerPlayer > MAX_SUBS_PER_PLAYER)) {
    throw Error(Msg() << "Invalid subs per player count: " << subsPerPlayer);
  }
  if (submarineConfigs.size() != subsPerPlayer) {
    throw Error(Msg() << "Sub config count (" << submarineConfigs.size()
                << ") does not equal subs per player count ("
                << subsPerPlayer << ")");
  }
  if (obstacles.size() > ((mapWidth * mapHeight) / 4)) {
    throw Error("Obstacle count may not exceed 1/4 of map area");
  }

  std::map<GameSetting::SettingType, unsigned> count;
  for (const GameSetting& setting : customSettings) {
    switch (setting.getType()) {
    case GameSetting::None:
      break;
    case GameSetting::MinPlayers:
    case GameSetting::MaxPlayers:
    case GameSetting::MaxTurns:
    case GameSetting::MapSize:
    case GameSetting::SubsPerPlayer:
      if (count.count(setting.getType())) {
        throw Error(Msg() << "Multiple " << setting.getName() << " settings");
      }
      count[setting.getType()] = 1;
      break;
    case GameSetting::SubStartLocation:
    case GameSetting::SubSurfaceTurnCount:
    case GameSetting::SubMaxShields:
    case GameSetting::SubMaxReactorDamage:
    case GameSetting::SubMaxSonarCharge:
    case GameSetting::SubMaxTorpedoCharge:
    case GameSetting::SubMaxMineCharge:
    case GameSetting::SubMaxSprintCharge:
    case GameSetting::SubTorpedoCount:
    case GameSetting::SubMineCount:
      if (!count.count(setting.getType())) {
        count[setting.getType()] = 1;
      } else {
        count[setting.getType()]++;
      }
      if (count[setting.getType()] > subsPerPlayer) {
        throw Error(Msg() << "Too many " << setting.getName() << " settings");
      }
      break;
    }
  }
}

//-----------------------------------------------------------------------------
static void setValue(unsigned& value,
                     const GameSetting& setting,
                     const unsigned minValue = 0,
                     const unsigned maxValue = 0)
{
  if (setting.getValueCount() != 1) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 1 value");
  }
  unsigned tmp = setting.getUnsignedValue();
  if ((tmp < minValue) || (maxValue && (tmp > maxValue))) {
    throw Error(Msg() << "Invalid " << setting.getName() << " value: "
                << setting.getStrValue());
  }
  value = tmp;
}

//-----------------------------------------------------------------------------
static void setValues(unsigned& value1,
                      unsigned& value2,
                      const GameSetting& setting,
                      const unsigned min1,
                      const unsigned min2,
                      const unsigned max1 = 0,
                      const unsigned max2 = 0)
{
  if (setting.getValueCount() != 2) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 2 values");
  }
  unsigned tmp1 = setting.getUnsignedValue(0);
  unsigned tmp2 = setting.getUnsignedValue(1);
  if ((tmp1 < min1) || (tmp2 < min2) ||
      (max1 && (tmp1 > max1)) || (max2 && (tmp2 > max2)))
  {
    throw Error(Msg() << "Invalid " << setting.getName() << " value: "
                << setting.getStrValue(0) << '|' << setting.getStrValue(1));
  }
  value1 = tmp1;
  value2 = tmp2;
}

//-----------------------------------------------------------------------------
static void setValues(unsigned& value,
                      Coordinate& coord,
                      const GameSetting& setting,
                      const unsigned minValue,
                      const unsigned maxValue,
                      const unsigned mapWidth,
                      const unsigned mapHeight)
{
  if (setting.getValueCount() != 2) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 3 values");
  }
  unsigned tmp1 = setting.getUnsignedValue(0);
  unsigned tmp2 = setting.getUnsignedValue(1);
  unsigned tmp3 = setting.getUnsignedValue(2);
  if ((tmp1 < minValue) || (maxValue && (tmp1 > maxValue)) || !tmp2 || !tmp3 ||
      (tmp2 > mapWidth) || (tmp3 > mapHeight))
  {
    throw Error(Msg() << "Invalid " << setting.getName() << " value: "
                << setting.getStrValue(0) << '|' << setting.getStrValue(1)
                << '|' << setting.getStrValue(2));
  }
  value = tmp1;
  coord.set(tmp2, tmp3);
}

//-----------------------------------------------------------------------------
GameConfig&
GameConfig::addCustomSetting(const GameSetting& setting) {
  const unsigned maxSubID = (submarineConfigs.size() - 1);
  Coordinate coord;
  unsigned x;
  unsigned y;

  switch (setting.getType()) {
  case GameSetting::None:
    throw Error(Msg() << "Invalid game setting: " << setting.getName());

  case GameSetting::MinPlayers:
    setValue(minPlayers, setting, 2);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::MaxPlayers:
    setValue(maxPlayers, setting, 2);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::MaxTurns:
    setValue(maxTurns, setting, MIN_TURN_COUNT);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::MapSize:
    setValues(mapWidth, mapHeight, setting,
              MIN_MAP_WIDTH, MIN_MAP_HEIGHT,
              MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::Obstacle:
    setValues(x, y, setting, 1, 1, mapWidth, mapHeight);
    obstacles.push_back(Coordinate(x, y));
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubsPerPlayer:
    setValue(subsPerPlayer, setting, 1, MAX_SUBS_PER_PLAYER);
    submarineConfigs.clear();
    for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
      submarineConfigs.push_back(std::make_unique<Submarine>(subID));
    }
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubStartLocation:
    setValues(x, coord, setting, 0, maxSubID, mapWidth, mapHeight);
    submarineConfigs[x].setStartLocation(coord);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubSurfaceTurnCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setSurfaceTurnCount(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxShields:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxShields(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxReactorDamage:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxReactorDamage(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxSonarCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSonarCharge(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxTorpedoCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxTorpedoCharge(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxMineCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxMineCharge(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMaxSprintCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSprintCharge(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubTorpedoCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setTorpedoCount(y);
    customSettings.push_back(setting);
    return (*this);

  case GameSetting::SubMineCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMineCount(y);
    return (*this);
  }

  throw Error(Msg() << "Unhandled game setting: " << setting.getName());
}

//-----------------------------------------------------------------------------
std::string
GameConfig::toMessage(const Version& serverVersion,
                      const std::string& title) const
{
  return CSVWriter('|', true) << serverVersion
                              << title
                              << mapWidth
                              << mapHeight
                              << settings.size();
}

} // namespace subsim
