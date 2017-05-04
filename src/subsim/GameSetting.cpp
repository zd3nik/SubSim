//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameSetting.h"
#include "utils/CSVReader.h"
#include "utils/CSVWriter.h"
#include "utils/Error.h"
#include "utils/Msg.h"
#include "utils/StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
std::string
GameSetting::typeName(const SettingType type) {
  switch (type) {
    case None:                return "None";
    case MinPlayers:          return "MinPlayers";
    case MaxPlayers:          return "MaxPlayers";
    case MaxTurns:            return "MaxTurns";
    case TurnTimeout:         return "TurnTimeout";
    case MapSize:             return "MapSize";
    case Obstacle:            return "Obstacle";
    case SubsPerPlayer:       return "SubsPerPlayer";
    case SubSize:             return "SubSize";
    case SubStartLocation:    return "SubStartLocation";
    case SubSurfaceTurnCount: return "SubSurfaceTurnCount";
    case SubMaxShields:       return "SubMaxShields";
    case SubMaxReactorDamage: return "SubMaxReactorDamage";
    case SubMaxSonarCharge:   return "SubMaxSonarCharge";
    case SubMaxTorpedoCharge: return "SubMaxTorpedoCharge";
    case SubMaxMineCharge:    return "SubMaxMineCharge";
    case SubMaxSprintCharge:  return "SubMaxSprintCharge";
    case SubTorpedoCount:     return "SubTorpedoCount";
    case SubMineCount:        return "SubMineCount";
  }
  throw Error(Msg() << "Invalid GameSetting type: "
              << static_cast<unsigned>(type));
}

//-----------------------------------------------------------------------------
GameSetting::SettingType
GameSetting::getType(const std::string& name) {
  static const std::string eMinPlayers          = typeName(MinPlayers);
  static const std::string eMaxPlayers          = typeName(MaxPlayers);
  static const std::string eMaxTurns            = typeName(MaxTurns);
  static const std::string eTurnTimeout         = typeName(TurnTimeout);
  static const std::string eMapSize             = typeName(MapSize);
  static const std::string eObstacle            = typeName(Obstacle);
  static const std::string eSubsPerPlayer       = typeName(SubsPerPlayer);
  static const std::string eSubSize             = typeName(SubSize);
  static const std::string eSubStartLocation    = typeName(SubStartLocation);
  static const std::string eSubSurfaceTurnCount = typeName(SubSurfaceTurnCount);
  static const std::string eSubMaxShields       = typeName(SubMaxShields);
  static const std::string eSubMaxReactorCharge = typeName(SubMaxReactorDamage);
  static const std::string eSubMaxSonarCharge   = typeName(SubMaxSonarCharge);
  static const std::string eSubMaxTorpedoCharge = typeName(SubMaxTorpedoCharge);
  static const std::string eSubMaxMineCharge    = typeName(SubMaxMineCharge);
  static const std::string eSubMaxSprintCharge  = typeName(SubMaxSprintCharge);
  static const std::string eSubTorpedoCount     = typeName(SubTorpedoCount);
  static const std::string eSubMineCount        = typeName(SubMineCount);

  if (iEqual(eMinPlayers, name)) {
    return MinPlayers;
  } else if (iEqual(eMaxPlayers, name)) {
    return MaxPlayers;
  } else if (iEqual(eMaxTurns, name)) {
    return MaxTurns;
  } else if (iEqual(eTurnTimeout, name)) {
    return TurnTimeout;
  } else if (iEqual(eMapSize, name)) {
    return MapSize;
  } else if (iEqual(eObstacle, name)) {
    return Obstacle;
  } else if (iEqual(eSubsPerPlayer, name)) {
    return SubsPerPlayer;
  } else if (iEqual(eSubSize, name)) {
    return SubSize;
  } else if (iEqual(eSubStartLocation, name)) {
    return SubStartLocation;
  } else if (iEqual(eSubSurfaceTurnCount, name)) {
    return SubSurfaceTurnCount;
  } else if (iEqual(eSubMaxShields, name)) {
    return SubMaxShields;
  } else if (iEqual(eSubMaxReactorCharge, name)) {
    return SubMaxReactorDamage;
  } else if (iEqual(eSubMaxSonarCharge, name)) {
    return SubMaxSonarCharge;
  } else if (iEqual(eSubMaxTorpedoCharge, name)) {
    return SubMaxTorpedoCharge;
  } else if (iEqual(eSubMaxMineCharge, name)) {
    return SubMaxMineCharge;
  } else if (iEqual(eSubMaxSprintCharge, name)) {
    return SubMaxSprintCharge;
  } else if (iEqual(eSubTorpedoCount, name)) {
    return SubTorpedoCount;
  } else if (iEqual(eSubMineCount, name)) {
    return SubMineCount;
  }

  throw Error(Msg() << "Unknown game setting: '" << name << "'");
}

//-----------------------------------------------------------------------------
GameSetting
GameSetting::fromMessage(const std::string& message) {
  std::vector<std::string> fields = CSVReader(message, '|', true).readCells();
  if (fields.empty()) {
    throw Error("Empty game setting message");
  } else if (fields.front() != "V") {
    throw Error(Msg() << "Invalid game setting message: " << message);
  } else if (fields.size() < 2) {
    throw Error(Msg() << "Incomplete game setting message: " << message);
  }

  GameSetting setting(getType(fields[1]));
  switch (setting.getType()) {
  case None:
    break;
  case MinPlayers:
  case MaxPlayers:
  case MaxTurns:
  case TurnTimeout:
  case SubsPerPlayer:
    if (fields.size() != 3) {
      throw Error(Msg() << "Expected 1 value in game setting message: "
                  << message);
    }
    setting.addValue(fields[2]);
    if (setting.getUnsignedValue(0, ~0U) == ~0U) {
      throw Error(Msg() << "Game setting requires a positive integer value: "
                  << message);
    }
    return setting;
  case SubSize:
  case SubSurfaceTurnCount:
  case SubMaxShields:
  case SubMaxReactorDamage:
  case SubMaxSonarCharge:
  case SubMaxTorpedoCharge:
  case SubMaxMineCharge:
  case SubMaxSprintCharge:
  case SubTorpedoCount:
  case SubMineCount:
    if (fields.size() != 4) {
      throw Error(Msg() << "Expected 2 values in game setting message: "
                  << message);
    }
    setting.addValue(fields[2]);
    setting.addValue(fields[3]);
    if ((setting.getUnsignedValue(0, ~0U) == ~0U) ||
        (setting.getUnsignedValue(1, ~0U) == ~0U))
    {
      throw Error(Msg() << "This game setting requires a submarine ID "
                  << "and a positive integer value: " << message);
    }
    return setting;
  case MapSize:
  case Obstacle:
    if (fields.size() != 4) {
      throw Error(Msg() << "Expected 2 values in game setting message: "
                  << message);
    }
    setting.addValue(fields[2]);
    setting.addValue(fields[3]);
    if (!setting.getUnsignedValue(0) || !setting.getUnsignedValue(1)) {
      throw Error(Msg() << "This game setting requires 2 non-zero "
                  << "positive integer values: " << message);
    }
    return setting;
  case SubStartLocation:
    if (fields.size() != 5) {
      throw Error(Msg() << "Expected 3 values in game setting message: "
                  << message);
    }
    setting.addValue(fields[2]);
    setting.addValue(fields[3]);
    setting.addValue(fields[4]);
    if ((setting.getUnsignedValue(0, ~0U) == ~0U) ||
        !setting.getUnsignedValue(1) ||
        !setting.getUnsignedValue(2))
    {
      throw Error(Msg() << "This game setting requires a submarine ID "
                  << "and coordinate values (ID|X|Y): " << message);
    }
    return setting;
  }

  throw Error(Msg() << "Unknown game setting message: " << message);
}

//-----------------------------------------------------------------------------
unsigned
GameSetting::getUnsignedValue(const unsigned idx, const unsigned def) const {
  return toUInt32(values[idx], def);
}

//-----------------------------------------------------------------------------
std::vector<unsigned>
GameSetting::getUnsignedValues(const unsigned def) const {
  std::vector<unsigned> tmp;
  tmp.reserve(values.size());
  for (const std::string& str : values) {
    tmp.push_back(toUInt32(str, def));
  }
  return std::move(tmp);
}

//-----------------------------------------------------------------------------
std::string
GameSetting::toMessage() const {
  CSVWriter msg('|', true);
  msg << 'V' << typeName(type);
  return msg.writeCells(values);
}

} // namespace subsim
