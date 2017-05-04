//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameConfig.h"
#include "utils/CSVWriter.h"
#include "utils/Error.h"
#include "utils/Msg.h"
#include "utils/Screen.h"

namespace subsim
{

//-----------------------------------------------------------------------------
GameConfig::GameConfig() {
  for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
    submarineConfigs.push_back(Submarine(subID));
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
void
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
    return;

  case GameSetting::MaxPlayers:
    setValue(maxPlayers, setting, 2);
    customSettings.push_back(setting);
    return;

  case GameSetting::MaxTurns:
    setValue(maxTurns, setting, MIN_TURN_COUNT);
    customSettings.push_back(setting);
    return;

  case GameSetting::TurnTimeout:
    setValue(turnTimeout, setting, MIN_TURN_TIMEOUT);
    customSettings.push_back(setting);
    return;

  case GameSetting::MapSize:
    setValues(mapWidth, mapHeight, setting,
              MIN_MAP_WIDTH, MIN_MAP_HEIGHT,
              MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    customSettings.push_back(setting);
    return;

  case GameSetting::Obstacle:
    setValues(x, y, setting, 1, 1, mapWidth, mapHeight);
    obstacles.push_back(Coordinate(x, y));
    customSettings.push_back(setting);
    return;

  case GameSetting::SubsPerPlayer:
    setValue(subsPerPlayer, setting, 1, MAX_SUBS_PER_PLAYER);
    submarineConfigs.clear();
    for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
      submarineConfigs.push_back(Submarine(subID));
    }
    customSettings.push_back(setting);
    return;

  case GameSetting::SubSize:
    setValues(x, y, setting, 0, MIN_SUB_SIZE, maxSubID, MAX_SUB_SIZE);
    submarineConfigs[x].setSize(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubStartLocation:
    setValues(x, coord, setting, 0, maxSubID, mapWidth, mapHeight);
    submarineConfigs[x].setLocation(coord);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubSurfaceTurnCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setSurfaceTurnCount(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxShields:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxShields(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxReactorDamage:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxReactorDamage(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxSonarCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSonarCharge(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxTorpedoCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxTorpedoCharge(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxMineCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxMineCharge(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMaxSprintCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSprintCharge(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubTorpedoCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setTorpedoCount(y);
    customSettings.push_back(setting);
    return;

  case GameSetting::SubMineCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMineCount(y);
    return;
  }

  throw Error(Msg() << "Unhandled game setting: " << setting.getName());
}

//-----------------------------------------------------------------------------
void
GameConfig::loadFrom(const DBRecord& record) {
  throw Error("TODO GameConfig::loadFrom");
}

//-----------------------------------------------------------------------------
void
GameConfig::print(const std::string& title, Coordinate& coord) const {
  Screen::print() << coord         << "Title       : " << title;
  Screen::print() << coord.south() << "Min Players : " << minPlayers;
  Screen::print() << coord.south() << "Max Players : " << maxPlayers;
  Screen::print() << coord.south() << "Max Turns   : "
                  << (maxTurns ? toStr(maxTurns) : "UNLIMITED");
  Screen::print() << coord.south() << "Turn Timeout: " << turnTimeout << " ms";
  Screen::print() << coord.south() << "Map Size    : "
                  << mapWidth << " x " << mapHeight;
  Screen::print() << coord.south() << "Subs/Player : " << subsPerPlayer;
  Screen::print() << coord.south() << "Obstacles   : " << obstacles.size();

  // TODO print customized settings - minus those shown above

  Screen::print() << coord.south().setX(1);
}

//-----------------------------------------------------------------------------
void
GameConfig::saveTo(DBRecord& record) const {
  throw Error("TODO GameConfig::saveTo");
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
  if (maxTurns && (maxTurns < MIN_TURN_COUNT)) {
    throw Error(Msg() << "Invalid max turn count: " << maxTurns);
  }
  if (turnTimeout && (turnTimeout < MIN_TURN_TIMEOUT)) {
    throw Error(Msg() << "Invalid turn timeout: " << turnTimeout);
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
  for (unsigned i = 0; i < submarineConfigs.size(); ++i) {
    const Submarine& sub = submarineConfigs[i];
    if (sub.getPlayerID() != ~0U) {
      throw Error(Msg() << "Sub config " << i << " has player ID "
                  << sub.getPlayerID());
    }
    if (sub.getObjectID() != i) {
      throw Error(Msg() << "Sub config " << i << " has object ID "
                  << sub.getObjectID());
    }
  }
  if (obstacles.size() > ((mapWidth * mapHeight) / 4)) {
    throw Error("Obstacle count may not exceed 1/4 of map area");
  }

  std::map<GameSetting::SettingType, unsigned> count;
  bool subConfigs = false;
  for (const GameSetting& setting : customSettings) {
    switch (setting.getType()) {
    case GameSetting::None:
    case GameSetting::Obstacle:
      break;
    case GameSetting::MinPlayers:
    case GameSetting::MaxPlayers:
    case GameSetting::MaxTurns:
    case GameSetting::TurnTimeout:
    case GameSetting::MapSize:
    case GameSetting::SubsPerPlayer:
      if (count.count(setting.getType())) {
        throw Error(Msg() << "Multiple " << setting.getName() << " settings");
      }
      if (subConfigs) {
        throw Error(Msg() << setting.getName()
                    << " setting must come before submarine settings");
      }
      count[setting.getType()] = 1;
      break;
    case GameSetting::SubSize:
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
      subConfigs = true;
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
std::string
GameConfig::toMessage(const Version& serverVersion,
                      const std::string& title) const
{
  return CSVWriter('|', true) << serverVersion
                              << title
                              << mapWidth
                              << mapHeight
                              << customSettings.size();
}

} // namespace subsim
