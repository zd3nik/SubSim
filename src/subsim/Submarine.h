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
private: // configurable variables
  Coordinate startLocation;
  unsigned surfaceTurnCount = 3;
  unsigned maxShields = 3;
  unsigned maxReactorDamage = 9;
  unsigned maxSonarCharge = 100;
  unsigned maxTorpedoCharge = 100;
  unsigned maxMineCharge = 3;
  unsigned maxSprintCharge = 9;
  unsigned torpedoCount = 9999;
  unsigned mineCount = 9999;

//-----------------------------------------------------------------------------
private: // runtime variables
  bool dead = false;
  bool detonated = false;
  unsigned surfaceTurns = 0; // turns remaining until surface maneuver complete
  unsigned shieldCount = maxShields;
  unsigned reactorDamage = 0;
  unsigned sonarCharge = 0;
  unsigned torpedoCharge = 0;
  unsigned mineCharge = 0;
  unsigned sprintCharge = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Submarine() noexcept = default;
  Submarine(Submarine&&) = delete;
  Submarine(const Submarine&) = delete;
  Submarine& operator=(Submarine&&) = delete;
  Submarine& operator=(const Submarine&) = delete;

  Submarine(const unsigned subID, const unsigned size = 100) noexcept;
  Submarine(const unsigned playerID,
            const unsigned subID,
            const Submarine& subTemplate) noexcept;

//-----------------------------------------------------------------------------
public: // static methods
  static std::string equipmentName(const Equipment);
  static Equipment getEquipment(const std::string& equipmentName);

//-----------------------------------------------------------------------------
public: // setters
  Submarine& setStartLocation(const Coordinate& coord) noexcept {
    startLocation = coord;
  }

  Submarine& setSurfaceTurnCount(const unsigned count) noexcept {
    surfaceTurnCount = count;
    return (*this);
  }

  Submarine& setMaxShields(const unsigned count) noexcept {
    maxShields = count;
    return (*this);
  }

  Submarine& setMaxReactorDamage(const unsigned count) noexcept {
    maxReactorDamage = count;
    return (*this);
  }

  Submarine& setMaxSonarCharge(const unsigned count) noexcept {
    maxSonarCharge = count;
    return (*this);
  }

  Submarine& setMaxTorpedoCharge(const unsigned count) noexcept {
    maxTorpedoCharge = count;
    return (*this);
  }

  Submarine& setMaxMineCharge(const unsigned count) noexcept {
    maxMineCharge = count;
    return (*this);
  }

  Submarine& setMaxSprintCharge(const unsigned count) noexcept {
    maxSprintCharge = count;
    return (*this);
  }

  Submarine& setTorpedoCount(const unsigned count) noexcept {
    torpedoCount = count;
    return (*this);
  }

  Submarine& setMineCount(const unsigned count) noexcept {
    mineCount = count;
    return (*this);
  }

//-----------------------------------------------------------------------------
public: // getters
  Coordinate getStartLocation() const noexcept {
    return startLocation;
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

//-----------------------------------------------------------------------------
typedef std::unique_ptr<Submarine> UniqueSubPtr;
typedef std::shared_ptr<Submarine> SharedSubPtr;

} // namespace subsim

#endif // SUBSIM_SUBMARINE_H
