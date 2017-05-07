//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_OBSTACLE_H
#define SUBSIM_OBSTACLE_H

#include "utils/Platform.h"
#include "Object.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Obstacle : public Object {
//-----------------------------------------------------------------------------
public: // constructors
  Obstacle(Obstacle&&) noexcept = default;
  Obstacle(const Obstacle&) noexcept = default;
  Obstacle& operator=(Obstacle&&) noexcept = default;
  Obstacle& operator=(const Obstacle&) noexcept = default;

  Obstacle() noexcept
    : Object(~0U, ~0U, ~0U, false)
  { }

//-----------------------------------------------------------------------------
public: // Object::Printable implementation
  std::string toString() const override {
    return ("Obstacle(coord " + getLocation().toString() + ")");
  }
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Obstacle> ObstaclePtr;

} // namespace subsim

#endif // SUBSIM_OBSTACLE_H
