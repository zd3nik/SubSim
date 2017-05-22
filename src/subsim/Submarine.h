//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SUBMARINE_H
#define SUBSIM_SUBMARINE_H

#include "utils/Platform.h"
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
  unsigned surfaceTurnCount = 3;
  unsigned maxShields = 3;
  unsigned maxReactorDamage = 9;
  unsigned maxSonarCharge = 100;
  unsigned maxTorpedoCharge = 100;
  unsigned maxMineCharge = 3;
  unsigned maxSprintCharge = 9;
  unsigned shieldCount = maxShields;
  unsigned torpedoCount = ~0U;
  unsigned mineCount = ~0U;
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
            const char mapChar,
            const unsigned subID,
            const Submarine& subTemplate) noexcept;

//-----------------------------------------------------------------------------
public: // Object::Printable implementation
  std::string toString() const override;

//-----------------------------------------------------------------------------
public: // static methods
  static std::string equipmentName(const Equipment);
  static Equipment getEquipment(const std::string& equipmentName);

//-----------------------------------------------------------------------------
public: // setters
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
  bool isActive() const noexcept {
    return !(dead | surfaceTurns);
  }

  bool isDead() const noexcept {
    return dead;
  }

  bool hasDetonated() const noexcept {
    return detonated;
  }

  bool getSurfaceTurns() const noexcept {
    return surfaceTurns;
  }

  unsigned getMaxShields() const noexcept {
    return maxShields;
  }

  unsigned getMaxReactorDamage() const noexcept {
    return maxReactorDamage;
  }

  unsigned getMaxSonarCharge() const noexcept {
    return maxSonarCharge;
  }

  unsigned getMaxTorpedoCharge() const noexcept {
    return maxTorpedoCharge;
  }

  unsigned getMaxMineCharge() const noexcept {
    return maxMineCharge;
  }

  unsigned getMaxSprintCharge() const noexcept {
    return maxSprintCharge;
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

  unsigned getSonarRange() const noexcept {
    return sonarCharge ? (sonarCharge + 1) : 0U;
  }

  unsigned getTorpedoCharge() const noexcept {
    return torpedoCharge;
  }

  unsigned getTorpedoRange() const noexcept {
    return torpedoCharge ? (torpedoCharge - 1) : 0U;
  }

  unsigned getMineCharge() const noexcept {
    return mineCharge;
  }

  unsigned getSprintCharge() const noexcept {
    return sprintCharge;
  }

  unsigned getSprintRange() const noexcept {
    return (sprintCharge >= 3) ? ((sprintCharge / 3) + 1) : 0U;
  }

  unsigned getTorpedoCount() const noexcept {
    return torpedoCount;
  }

  unsigned getMineCount() const noexcept {
    return mineCount;
  }

//-----------------------------------------------------------------------------
public: // command methods
  unsigned ping() noexcept;
  bool charge(const Equipment) noexcept;
  bool fire(const unsigned distance) noexcept;
  bool mine() noexcept;
  bool sprint(const unsigned distance) noexcept;
  bool surface() noexcept;

//-----------------------------------------------------------------------------
public: // methods
  void kill() noexcept;
  void repair() noexcept;
  void takeHits(const unsigned hits) noexcept;
  void takeReactorDamage(const unsigned damage) noexcept;
  void takeReactorStrain(const unsigned strain) noexcept;
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Submarine> SubmarinePtr;

} // namespace subsim

#endif // SUBSIM_SUBMARINE_H
