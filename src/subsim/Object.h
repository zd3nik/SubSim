//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_OBJECT_H
#define SUBSIM_OBJECT_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Object : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  unsigned playerID;
  unsigned objectID;
  unsigned size;
  bool permanent;
  Coordinate location;

//-----------------------------------------------------------------------------
public: // constructors
  Object() = delete;
  Object(Object&&) noexcept = default;
  Object(const Object&) noexcept = default;
  Object& operator=(Object&&) noexcept = default;
  Object& operator=(const Object&) noexcept = default;

  Object(const unsigned playerID,
         const unsigned objectID,
         const unsigned size,
         const bool permanent) noexcept
    : playerID(playerID),
      objectID(objectID),
      size(size),
      permanent(permanent)
  { }

//-----------------------------------------------------------------------------
public: // setters
  void setLocation(const Coordinate& coord) noexcept {
    location = coord;
  }

//-----------------------------------------------------------------------------
public: // getters
  unsigned getPlayerID() const noexcept {
    return playerID;
  }

  unsigned getObjectID() const noexcept {
    return objectID;
  }

  unsigned getSize() const noexcept {
    return size;
  }

  unsigned isPermanent() const noexcept {
    return permanent;
  }

  const Coordinate& getLocation() const noexcept {
    return location;
  }

  Coordinate& getLocation() noexcept {
    return location;
  }

//-----------------------------------------------------------------------------
protected: // setters
  void setSize(const unsigned value) noexcept {
    size = value;
  }
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Object> ObjectPtr;

} // namespace subsim

#endif // SUBSIM_OBJECT_H
