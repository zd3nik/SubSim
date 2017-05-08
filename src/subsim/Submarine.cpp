//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Submarine.h"
#include "utils/Error.h"
#include "utils/Msg.h"

namespace subsim
{

//-----------------------------------------------------------------------------
Submarine::Submarine(const unsigned subID,
                     const unsigned size) noexcept
  : Object(~0U, subID, size, false)
{ }

//-----------------------------------------------------------------------------
Submarine::Submarine(const unsigned playerID,
                     const unsigned subID,
                     const Submarine& sub) noexcept
  : Object(playerID, subID, sub.getSize(), false),
    surfaceTurnCount(sub.surfaceTurnCount),
    maxShields(sub.maxShields),
    maxReactorDamage(sub.maxReactorDamage),
    maxSonarCharge(sub.maxSonarCharge),
    maxTorpedoCharge(sub.maxTorpedoCharge),
    maxMineCharge(sub.maxMineCharge),
    maxSprintCharge(sub.maxSprintCharge),
    shieldCount(maxShields),
    torpedoCount(sub.torpedoCount),
    mineCount(sub.mineCount)
{ }

//-----------------------------------------------------------------------------
std::string
Submarine::equipmentName(const Equipment equip) {
  switch (equip) {
    case None:    return "None";
    case Sonar:   return "Sonar";
    case Torpedo: return "Torpedo";
    case Mine:    return "Mine";
    case Sprint:  return "Sprint";
  }
  throw Error(Msg() << "Invalid submarine equipment type: "
              << static_cast<unsigned>(equip));
}

//-----------------------------------------------------------------------------
Submarine::Equipment
Submarine::getEquipment(const std::string& name) {
  static const std::string eNone    = equipmentName(None);
  static const std::string eSonar   = equipmentName(Sonar);
  static const std::string eTorpedo = equipmentName(Torpedo);
  static const std::string eMine    = equipmentName(Mine);
  static const std::string eSprint  = equipmentName(Sprint);

  if (iEqual(eNone, name)) {
    return None;
  } else if (iEqual(eSonar, name)) {
    return Sonar;
  } else if (iEqual(eTorpedo, name)) {
    return Torpedo;
  } else if (iEqual(eMine, name)) {
    return Mine;
  } else if (iEqual(eSprint, name)) {
    return Sprint;
  }

  throw Error(Msg() << "Unknown equipment name: '" << name << "'");
}

//-----------------------------------------------------------------------------
std::string
Submarine::toString() const {
  return Msg() << "Submarine(playerID " << getPlayerID() << ", subID "
               <<  getObjectID() << ", coord " << getLocation() << ")";
}

//-----------------------------------------------------------------------------
unsigned
Submarine::ping() noexcept {
  if (sonarCharge) {
    const unsigned range = (sonarCharge + 1);
    sonarCharge = 0;
    return range;
  }
  return 0;
}

//-----------------------------------------------------------------------------
bool
Submarine::charge(const Equipment equip) noexcept {
  switch (equip) {
  case None:
    takeReactorDamage(1);
    return true;
  case Sonar:
    if (++sonarCharge >= maxSonarCharge) {
      takeReactorDamage(1);
    }
    return true;
  case Torpedo:
    if (++torpedoCharge >= maxTorpedoCharge) {
      takeReactorDamage(1);
    }
    return true;
  case Mine:
    if (++mineCharge >= maxMineCharge) {
      takeReactorDamage(1);
    }
    return true;
  case Sprint:
    if (++sprintCharge >= maxSprintCharge) {
      takeReactorDamage(1);
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
Submarine::fire(const unsigned distance) noexcept {
  if (torpedoCharge > distance) {
    torpedoCharge = 0;
    return true;
  }
  torpedoCharge = 0;
  return false;
}

//-----------------------------------------------------------------------------
bool
Submarine::mine() noexcept {
  if (mineCharge == maxMineCharge) {
    mineCharge = 0;
    return true;
  }
  mineCharge = 0;
  return false;
}

//-----------------------------------------------------------------------------
bool
Submarine::sprint(const unsigned distance) noexcept {
  const unsigned chargeDistance = ((sprintCharge / 3) + 1);
  sprintCharge = 0;
  if (chargeDistance >= distance) {
    takeReactorStrain(chargeDistance);
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
Submarine::surface() noexcept {
  if (!surfaceTurns) {
    surfaceTurns = surfaceTurnCount;
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void
Submarine::kill() noexcept {
  dead = true;
}

//-----------------------------------------------------------------------------
void
Submarine::repair() noexcept {
  if (!dead && surfaceTurns) {
    if ((--surfaceTurns == 0) && (shieldCount < maxShields)) {
      shieldCount++;
    }
  }
}

//-----------------------------------------------------------------------------
void
Submarine::takeHits(const unsigned hits) noexcept {
  if (hits > shieldCount) {
    shieldCount = 0;
    dead = true;
  } else {
    shieldCount -= hits;
  }
}

//-----------------------------------------------------------------------------
void
Submarine::takeReactorDamage(const unsigned damage) noexcept {
  if ((reactorDamage += damage) >= maxReactorDamage) {
    dead = detonated = true;
  }
}

//-----------------------------------------------------------------------------
void
Submarine::takeReactorStrain(const unsigned strain) noexcept {
  if ((reactorDamage + strain) >= maxReactorDamage) {
    reactorDamage += strain;
    dead = detonated = true;
  }
}

} // namespace subsim
