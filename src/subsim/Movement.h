//-----------------------------------------------------------------------------
// Movement.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_MOVEMENT_H
#define SUBSIM_MOVEMENT_H

#include "Platform.h"

namespace subsim
{

//-----------------------------------------------------------------------------
enum Direction {
  North,
  East,
  South,
  West
};

//-----------------------------------------------------------------------------
class Movement {
//-----------------------------------------------------------------------------
private: // variables
  Direction direction;
  unsigned distance;

//-----------------------------------------------------------------------------
public: // constructors
  Movement() = delete;
  Movement(Movement&&) noexcept = default;
  Movement(const Movement&) noexcept = default;
  Movement& operator=(Movement&&) noexcept = default;
  Movement& operator=(const Movement&) noexcept = default;

  explicit Movement(const Direction direction,
                    const unsigned distance) noexcept
    : direction(direction),
      distance(distance)
  { }

//-----------------------------------------------------------------------------
public: // methods
  Direction getDirection() const noexcept {
    return direction;
  }

  unsigned getDistance() const noexcept {
    return distance;
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  bool operator==(const Movement& other) const noexcept {
    return ((direction == other.direction) && (distance == other.distance));
  }

  bool operator!=(const Movement& other) const noexcept {
    return ((direction != other.direction) || (distance != other.distance));
  }
};

} // namespace subsim

#endif // SUBSIM_MOVEMENT_H
