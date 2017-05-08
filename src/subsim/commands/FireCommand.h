//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_FIRE_COMMAND_H
#define SUBSIM_FIRE_COMMAND_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Msg.h"
#include "Command.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class FireCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'F';

//-----------------------------------------------------------------------------
private: // variables
  Coordinate dest;

//-----------------------------------------------------------------------------
public: // constructors
  FireCommand() = delete;
  FireCommand(FireCommand&&) noexcept = default;
  FireCommand(const FireCommand&) noexcept = default;
  FireCommand& operator=(FireCommand&&) noexcept = default;
  FireCommand& operator=(const FireCommand&) noexcept = default;

  FireCommand(const unsigned playerID, const Input& input)
    : Command(Command::FireTorpedo, playerID, input)
  {
    if (input.getFieldCount() != 5) {
      throw Error("Fire command requires 4 values");
    }
    dest.set(input.getUInt(3), input.getUInt(4));
    if (!dest) {
      throw Error("Invalid coordinate in command: " + input.getLine());
    }
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID()
                     << dest.getX() << dest.getY();
  }

//-----------------------------------------------------------------------------
public: // getters
  Coordinate getDestination() const noexcept {
    return dest;
  }
};

} // namespace subsim

#endif // SUBSIM_FIRE_COMMAND_H
