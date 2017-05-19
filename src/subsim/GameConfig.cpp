//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameConfig.h"
#include "utils/CSVReader.h"
#include "utils/CSVWriter.h"
#include "utils/Error.h"
#include "utils/Msg.h"
#include "utils/Screen.h"

namespace subsim
{

//-----------------------------------------------------------------------------
static const GameSetting::SettingType _None                = GameSetting::None;
static const GameSetting::SettingType _MinPlayers          = GameSetting::MinPlayers;
static const GameSetting::SettingType _MaxPlayers          = GameSetting::MaxPlayers;
static const GameSetting::SettingType _MaxTurns            = GameSetting::MaxTurns;
static const GameSetting::SettingType _TurnTimeout         = GameSetting::TurnTimeout;
static const GameSetting::SettingType _MapSize             = GameSetting::MapSize;
static const GameSetting::SettingType _Obstacle            = GameSetting::Obstacle;
static const GameSetting::SettingType _SubsPerPlayer       = GameSetting::SubsPerPlayer;
static const GameSetting::SettingType _SubSize             = GameSetting::SubSize;
static const GameSetting::SettingType _SubStartLocation    = GameSetting::SubStartLocation;
static const GameSetting::SettingType _SubSurfaceTurnCount = GameSetting::SubSurfaceTurnCount;
static const GameSetting::SettingType _SubMaxShields       = GameSetting::SubMaxShields;
static const GameSetting::SettingType _SubMaxReactorDamage = GameSetting::SubMaxReactorDamage;
static const GameSetting::SettingType _SubMaxSonarCharge   = GameSetting::SubMaxSonarCharge;
static const GameSetting::SettingType _SubMaxTorpedoCharge = GameSetting::SubMaxTorpedoCharge;
static const GameSetting::SettingType _SubMaxMineCharge    = GameSetting::SubMaxMineCharge;
static const GameSetting::SettingType _SubMaxSprintCharge  = GameSetting::SubMaxSprintCharge;
static const GameSetting::SettingType _SubTorpedoCount     = GameSetting::SubTorpedoCount;
static const GameSetting::SettingType _SubMineCount        = GameSetting::SubMineCount;

//-----------------------------------------------------------------------------
GameConfig::GameConfig() {
  for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
    submarineConfigs.push_back(Submarine(subID));
  }
}

//-----------------------------------------------------------------------------
void
GameConfig::print(const std::string& title, Coordinate& coord) const {
  Screen::print() << coord         << "Title: " << title;
  if (obstacles.size()) {
    Screen::print() << coord.south() << "Obstacle Count: " << obstacles.size();
  }
  for (const GameSetting& setting : customSettings) {
    if (setting.getType() != GameSetting::Obstacle) {
      Screen::print() << coord.south() << setting;
    }
  }

  Screen::print() << coord.south().setX(1);
}

//-----------------------------------------------------------------------------
static void setValue(unsigned& value,
                     const GameSetting& setting,
                     const unsigned minValue = 0,
                     const unsigned maxValue = ~0U)
{
  if (setting.getValueCount() != 1) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 1 value");
  }
  unsigned tmp = setting.getUnsignedValue();
  if ((tmp < minValue) || (tmp > maxValue)) {
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
                      const unsigned max1 = ~0U,
                      const unsigned max2 = ~0U)
{
  if (setting.getValueCount() != 2) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 2 values");
  }
  unsigned tmp1 = setting.getUnsignedValue(0);
  unsigned tmp2 = setting.getUnsignedValue(1);
  if ((tmp1 < min1) || (tmp2 < min2) ||
      (tmp1 > max1) || (tmp2 > max2))
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
  if (setting.getValueCount() != 3) {
    throw Error(Msg() << "Setting " << setting.getName()
                << " requires 3 values");
  }
  unsigned tmp1 = setting.getUnsignedValue(0);
  unsigned tmp2 = setting.getUnsignedValue(1);
  unsigned tmp3 = setting.getUnsignedValue(2);
  if ((tmp1 < minValue) || (tmp1 > maxValue) || !tmp2 || !tmp3 ||
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
GameConfig::addSetting(const GameSetting& setting) {
  const unsigned maxSubID = (submarineConfigs.size() - 1);
  Coordinate coord;
  unsigned x;
  unsigned y;

  switch (setting.getType()) {
  case _None:
    throw Error(Msg() << "Invalid game setting: " << setting.getName());

  case _MinPlayers:
    setValue(minPlayers, setting, 2);
    if (minPlayers != DEFAULT_MIN_PLAYERS) {
      customSettings.push_back(setting);
    }
    return;

  case _MaxPlayers:
    setValue(maxPlayers, setting, minPlayers);
    if (maxPlayers != DEFAULT_MAX_PLAYERS) {
      customSettings.push_back(setting);
    }
    return;

  case _MaxTurns:
    setValue(maxTurns, setting, MIN_TURN_COUNT);
    if (maxTurns != DEFAULT_MAX_TURNS) {
      customSettings.push_back(setting);
    }
    return;

  case _TurnTimeout:
    setValue(turnTimeout, setting, MIN_TURN_TIMEOUT);
    if (turnTimeout != DEFAULT_TURN_TIMEOUT) {
      customSettings.push_back(setting);
    }
    return;

  case _MapSize:
    setValues(mapWidth, mapHeight, setting,
              MIN_MAP_WIDTH, MIN_MAP_HEIGHT,
              MAX_MAP_WIDTH, MAX_MAP_HEIGHT);
    if ((mapWidth != DEFAULT_MAP_WIDTH) || (mapHeight != DEFAULT_MAP_HEIGHT)) {
      customSettings.push_back(setting);
    }
    return;

  case _Obstacle:
    setValues(x, y, setting, 1, 1, mapWidth, mapHeight);
    obstacles.push_back(Coordinate(x, y));
    customSettings.push_back(setting);
    return;

  case _SubsPerPlayer:
    setValue(subsPerPlayer, setting, 1, MAX_SUBS_PER_PLAYER);
    submarineConfigs.clear();
    for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
      submarineConfigs.push_back(Submarine(subID));
    }
    if (subsPerPlayer != DEFAULT_SUBS_PER_PLAYER) {
      customSettings.push_back(setting);
    }
    return;

  case _SubSize:
    setValues(x, y, setting, 0, MIN_SUB_SIZE, maxSubID, MAX_SUB_SIZE);
    submarineConfigs[x].setSize(y);
    customSettings.push_back(setting);
    return;

  case _SubStartLocation:
    setValues(x, coord, setting, 0, maxSubID, mapWidth, mapHeight);
    submarineConfigs[x].setLocation(coord);
    customSettings.push_back(setting);
    return;

  case _SubSurfaceTurnCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setSurfaceTurnCount(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxShields:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxShields(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxReactorDamage:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxReactorDamage(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxSonarCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSonarCharge(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxTorpedoCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxTorpedoCharge(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxMineCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxMineCharge(y);
    customSettings.push_back(setting);
    return;

  case _SubMaxSprintCharge:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMaxSprintCharge(y);
    customSettings.push_back(setting);
    return;

  case _SubTorpedoCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setTorpedoCount(y);
    customSettings.push_back(setting);
    return;

  case _SubMineCount:
    setValues(x, y, setting, 0, 0, maxSubID);
    submarineConfigs[x].setMineCount(y);
    customSettings.push_back(setting);
    return;
  }

  throw Error(Msg() << "Unhandled game setting: " << setting.getName());
}

//-----------------------------------------------------------------------------
void
GameConfig::get(const GameSetting::SettingType type, const DBRecord& record) {
  auto strings = record.getStrings(GameSetting::typeName(type));
  for (const std::string& str : strings) {
    auto values = CSVReader(str, '|', true).readCells();
    addSetting(GameSetting(type, values));
  }
}

//-----------------------------------------------------------------------------
void
GameConfig::loadFrom(const DBRecord& record) {
  minPlayers = record.getUInt(GameSetting::typeName(_MinPlayers));
  if (minPlayers != DEFAULT_MIN_PLAYERS) {
    customSettings.push_back(GameSetting(_MinPlayers, minPlayers));
  }

  maxPlayers = record.getUInt(GameSetting::typeName(_MaxPlayers));
  if (maxPlayers != DEFAULT_MAX_PLAYERS) {
    customSettings.push_back(GameSetting(_MaxPlayers, maxPlayers));
  }

  maxTurns = record.getUInt(GameSetting::typeName(_MaxTurns));
  if (maxTurns != DEFAULT_MAX_TURNS) {
    customSettings.push_back(GameSetting(_MaxTurns, maxTurns));
  }

  turnTimeout = record.getUInt(GameSetting::typeName(_TurnTimeout));
  if (turnTimeout != DEFAULT_TURN_TIMEOUT) {
    customSettings.push_back(GameSetting(_TurnTimeout, turnTimeout));
  }

  subsPerPlayer = record.getUInt(GameSetting::typeName(_SubsPerPlayer));
  submarineConfigs.clear();
  for (unsigned subID = 0; subID < subsPerPlayer; ++subID) {
    submarineConfigs.push_back(Submarine(subID));
  }
  if (subsPerPlayer != DEFAULT_SUBS_PER_PLAYER) {
    customSettings.push_back(GameSetting(_SubsPerPlayer, subsPerPlayer));
  }

  auto coords = record.getUInts(GameSetting::typeName(_MapSize));
  if (coords.size()) {
    addSetting(GameSetting(_MapSize, coords));
  }

  get(_Obstacle, record);
  get(_SubSize, record);
  get(_SubStartLocation, record);
  get(_SubSurfaceTurnCount, record);
  get(_SubMaxShields, record);
  get(_SubMaxReactorDamage, record);
  get(_SubMaxSonarCharge, record);
  get(_SubMaxTorpedoCharge, record);
  get(_SubMaxMineCharge, record);
  get(_SubMaxSprintCharge, record);
  get(_SubTorpedoCount, record);
  get(_SubMineCount, record);
}

//-----------------------------------------------------------------------------
void
GameConfig::store(const GameSetting::SettingType type, DBRecord& record) const {
  std::string name = GameSetting::typeName(type);
  record.clear(name);

  for (const GameSetting& setting : customSettings) {
    if (setting.getType() == type) {
      CSVWriter csv('|', true);
      csv.writeCells(setting.getStrValues());
      record.addString(name, csv.toString());
    }
  }
}

//-----------------------------------------------------------------------------
void
GameConfig::saveTo(DBRecord& record) const {
  record.setUInt(GameSetting::typeName(_MinPlayers), minPlayers);
  record.setUInt(GameSetting::typeName(_MaxPlayers), maxPlayers);
  record.setUInt(GameSetting::typeName(_MaxTurns), maxTurns);
  record.setUInt(GameSetting::typeName(_TurnTimeout), turnTimeout);
  record.setUInt(GameSetting::typeName(_SubsPerPlayer), subsPerPlayer);

  record.clear(GameSetting::typeName(_MapSize));
  record.addUInt(GameSetting::typeName(_MapSize), mapWidth);
  record.addUInt(GameSetting::typeName(_MapSize), mapHeight);

  store(_Obstacle, record);
  store(_SubSize, record);
  store(_SubStartLocation, record);
  store(_SubSurfaceTurnCount, record);
  store(_SubMaxShields, record);
  store(_SubMaxReactorDamage, record);
  store(_SubMaxSonarCharge, record);
  store(_SubMaxTorpedoCharge, record);
  store(_SubMaxMineCharge, record);
  store(_SubMaxSprintCharge, record);
  store(_SubTorpedoCount, record);
  store(_SubMineCount, record);
}

//-----------------------------------------------------------------------------
void
GameConfig::validate() const {
  if (minPlayers < 2) {
    throw Error(Msg() << "Invalid min player count: " << minPlayers);
  }
  if (maxPlayers && (maxPlayers < minPlayers)) {
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
    const Coordinate coord = sub.getLocation();
    if (coord) {
      if ((coord.getX() > mapWidth) || (coord.getY() > mapHeight)) {
        throw Error(Msg() << "Sub config " << i << " has invalid location: "
                    << coord);
      }
      for (const Coordinate& obstacle: obstacles) {
        if (coord == obstacle) {
          throw Error(Msg() << "Sub config " << i << " coordinate "
                      << obstacle << " is blocked by an obstacle");
        }
      }
    }
  }
  if (obstacles.size() > ((mapWidth * mapHeight) / 4)) {
    throw Error("Obstacle count may not exceed 1/4 of map area");
  }

  std::map<GameSetting::SettingType, unsigned> count;
  bool subConfigs = false;
  for (const GameSetting& setting : customSettings) {
    switch (setting.getType()) {
    case _None:
    case _Obstacle:
      break;
    case _MinPlayers:
    case _MaxPlayers:
    case _MaxTurns:
    case _TurnTimeout:
    case _MapSize:
      if (count.count(setting.getType())) {
        throw Error(Msg() << "Multiple " << setting.getName() << " settings");
      }
      count[setting.getType()] = 1;
      break;
    case _SubsPerPlayer:
      if (count.count(setting.getType())) {
        throw Error(Msg() << "Multiple " << setting.getName() << " settings");
      }
      if (subConfigs) {
        throw Error(Msg() << setting.getName()
                    << " setting must come before submarine settings");
      }
      count[setting.getType()] = 1;
      break;
    case _SubSize:
    case _SubStartLocation:
    case _SubSurfaceTurnCount:
    case _SubMaxShields:
    case _SubMaxReactorDamage:
    case _SubMaxSonarCharge:
    case _SubMaxTorpedoCharge:
    case _SubMaxMineCharge:
    case _SubMaxSprintCharge:
    case _SubTorpedoCount:
    case _SubMineCount:
      subConfigs = true;
      if (!count.count(setting.getType())) {
        count[setting.getType()] = 1;
      } else {
        count[setting.getType()]++;
      }
      if (count[setting.getType()] > subsPerPlayer) {
        throw Error(Msg() << "Too many " << setting.getName() << " settings");
      }
      if (setting.getUnsignedValue(0, ~0U) >= subsPerPlayer) {
        throw Error(Msg() << "Invalid sub ID in " << setting.toMessage());
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
  return Msg('C') << serverVersion
                  << title
                  << mapWidth
                  << mapHeight
                  << customSettings.size();
}

} // namespace subsim
