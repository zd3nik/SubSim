//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SUBMARINE_H
#define SUBSIM_SUBMARINE_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Movement.h"
#include "Object.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Submarine : public Object {
//-----------------------------------------------------------------------------
public: // enums
  enum Equipment {
    None,
    Sonar,
    Torpedo,
    Mine,
    Sprint
  };

//-----------------------------------------------------------------------------
private: // variables
  Coordinate location;
  unsigned surfaceTurnCount = 3;
  unsigned maxShields = 3;
  unsigned maxReactorDamage = 9;
  unsigned maxSonarCharge = 100;
  unsigned maxTorpedoCharge = 100;
  unsigned maxMineCharge = 3;
  unsigned maxSprintCharge = 9;
  unsigned shieldCount = maxShields;
  unsigned torpedoCount = 9999;
  unsigned mineCount = 9999;
  unsigned surfaceTurns = 0; // turns remaining until surface maneuver complete
  unsigned reactorDamage = 0;
  unsigned sonarCharge = 0;
  unsigned torpedoCharge = 0;
  unsigned mineCharge = 0;
  unsigned sprintCharge = 0;
  bool dead = false;
  bool detonated = false;

//-----------------------------------------------------------------------------
public: // constructors
  Submarine() noexcept = default;
  Submarine(Submarine&&) noexcept = default;
  Submarine(const Submarine&) noexcept = default;
  Submarine& operator=(Submarine&&) noexcept = default;
  Submarine& operator=(const Submarine&) noexcept = default;

  Submarine(const unsigned subID = 0, const unsigned size = 100) noexcept;
  Submarine(const unsigned playerID,
            const unsigned subID,
            const Submarine& subTemplate) noexcept;

//-----------------------------------------------------------------------------
public: // Object::Printable implementation
  std::string toString() const override {
    return ("Submarine(" + toStr(getObjectID()) + ")");
  }

//-----------------------------------------------------------------------------
public: // static methods
  static std::string equipmentName(const Equipment);
  static Equipment getEquipment(const std::string& equipmentName);

//-----------------------------------------------------------------------------
public: // setters
  void setLocation(const Coordinate& coord) noexcept {
    location = coord;
  }

  void setSize(const unsigned value) noexcept {
    Object::setSize(value);
  }

  void setSurfaceTurnCount(const unsigned count) noexcept {
    surfaceTurnCount = count;
  }

  void setMaxShields(const unsigned count) noexcept {
    maxShields = count;
  }

  void setMaxReactorDamage(const unsigned count) noexcept {
    maxReactorDamage = count;
  }

  void setMaxSonarCharge(const unsigned count) noexcept {
    maxSonarCharge = count;
  }

  void setMaxTorpedoCharge(const unsigned count) noexcept {
    maxTorpedoCharge = count;
  }

  void setMaxMineCharge(const unsigned count) noexcept {
    maxMineCharge = count;
  }

  void setMaxSprintCharge(const unsigned count) noexcept {
    maxSprintCharge = count;
  }

  void setTorpedoCount(const unsigned count) noexcept {
    torpedoCount = count;
  }

  void setMineCount(const unsigned count) noexcept {
    mineCount = count;
  }

//-----------------------------------------------------------------------------
public: // getters
  Coordinate getLocation() const noexcept {
    return location;
  }

  bool isDead() const noexcept {
    return dead;
  }

  bool hasDetonated() const noexcept {
    return detonated;
  }

  bool isSurfaced() const noexcept {
    return surfaceTurns;
  }

  unsigned getShieldCount() const noexcept {
    return shieldCount;
  }

  unsigned getReactorDamage() const noexcept {
    return reactorDamage;
  }

  unsigned getSonarCharge() const noexcept {
    return sonarCharge;
  }

  unsigned getTorpedoCharge() const noexcept {
    return torpedoCharge;
  }

  unsigned getMineCharge() const noexcept {
    return mineCharge;
  }

  unsigned getSprintCharge() const noexcept {
    return sprintCharge;
  }

  unsigned getTorpedoCount() const noexcept {
    return torpedoCount;
  }

  unsigned getMineCount() const noexcept {
    return mineCount;
  }

//-----------------------------------------------------------------------------
public: // command methods
  void surface() noexcept;
  bool charge(const Equipment) noexcept;
  unsigned ping() noexcept;
  bool fire(const unsigned distance) noexcept;
  bool mine() noexcept;
  bool sprint(const unsigned distance) noexcept;

//-----------------------------------------------------------------------------
public: // methods
  bool takeHits(const unsigned hits) noexcept;
  bool takeReactorDamage(const unsigned damage) noexcept;
  bool takeReactorStrain(const unsigned strain) noexcept;
};

} // namespace subsim

#endif // SUBSIM_SUBMARINE_H
