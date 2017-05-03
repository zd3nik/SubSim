//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_OBJECT_H
#define SUBSIM_OBJECT_H

#include "utils/Platform.h"
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
  bool mobile;

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
