//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_MINE_H
#define SUBSIM_MINE_H

#include "utils/Platform.h"
#include "utils/Msg.h"
#include "Object.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Mine : public Object {
//-----------------------------------------------------------------------------
public: // constructors
  Mine(Mine&&) noexcept = default;
  Mine(const Mine&) noexcept = default;
  Mine& operator=(Mine&&) noexcept = default;
  Mine& operator=(const Mine&) noexcept = default;

  Mine(const unsigned playerID, const char mapChar) noexcept
    : Object(mapChar, playerID, ~0U, 10, false)
  { }

//-----------------------------------------------------------------------------
public: // Object::Printable implementation
  std::string toString() const override {
    return Msg() << "Mine(playerID " << getPlayerID() << ", coord "
                 << getLocation() << ")";
  }
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Mine> MinePtr;

} // namespace subsim

#endif // SUBSIM_MINE_H
