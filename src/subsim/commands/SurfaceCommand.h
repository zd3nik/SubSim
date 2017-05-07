//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SURFACE_COMMAND_H
#define SUBSIM_SURFACE_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Msg.h"
#include "Command.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class SurfaceCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'U';

//-----------------------------------------------------------------------------
public: // constructors
  SurfaceCommand() = delete;
  SurfaceCommand(SurfaceCommand&&) noexcept = default;
  SurfaceCommand(const SurfaceCommand&) noexcept = default;
  SurfaceCommand& operator=(SurfaceCommand&&) noexcept = default;
  SurfaceCommand& operator=(const SurfaceCommand&) noexcept = default;

  SurfaceCommand(const unsigned playerID, const Input& input)
    : Command(Command::DeploySurface, playerID, input)
  {
    if (input.getFieldCount() != 3) {
      throw Error("Surface command requires 2 values");
    }
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID();
  }
};

} // namespace subsim

#endif // SUBSIM_SURFACE_COMMAND_H
