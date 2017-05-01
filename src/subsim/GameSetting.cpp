//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameSetting.h"
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
  case MapSize:             return "MapSize";
  case Obstacle:            return "Obstacle";
  case SubsPerPlayer:       return "SubsPerPlayer";
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
  static const std::string eMapSize             = typeName(MapSize);
  static const std::string eObstacle            = typeName(Obstacle);
  static const std::string eSubsPerPlayer       = typeName(SubsPerPlayer);
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
  } else if (iEqual(eMapSize, name)) {
    return MapSize;
  } else if (iEqual(eObstacle, name)) {
    return Obstacle;
  } else if (iEqual(eSubsPerPlayer, name)) {
    return SubsPerPlayer;
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
