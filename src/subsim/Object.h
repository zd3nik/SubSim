//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_OBJECT_H
#define SUBSIM_OBJECT_H

#include "utils/Platform.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Object {
//-----------------------------------------------------------------------------
private: // variables
  const unsigned playerID;
  const unsigned objectID;
  const unsigned size;
  const bool mobile;

//-----------------------------------------------------------------------------
public: // constructors
  Object() = delete;
  Object(Object&&) = delete;
  Object(const Object&) = delete;
  Object& operator=(Object&&) = delete;
  Object& operator=(const Object&) = delete;

  Object(const unsigned playerID,
         const unsigned objectID,
         const unsigned size,
         const bool mobile) noexcept
    : playerID(playerID),
      objectID(objectID),
      size(size),
      mobile(mobile)
  { }

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

  unsigned isMobile() const noexcept {
    return mobile;
  }
};

} // namespace subsim

#endif // SUBSIM_OBJECT_H
