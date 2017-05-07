//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_PING_COMMAND_H
#define SUBSIM_PING_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Msg.h"
#include "Command.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class PingCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'P';

//-----------------------------------------------------------------------------
public: // constructors
  PingCommand() = delete;
  PingCommand(PingCommand&&) noexcept = default;
  PingCommand(const PingCommand&) noexcept = default;
  PingCommand& operator=(PingCommand&&) noexcept = default;
  PingCommand& operator=(const PingCommand&) noexcept = default;

  PingCommand(const unsigned playerID, const Input& input)
    : Command(Command::DeployPing, playerID, input)
  {
    if (input.getFieldCount() != 3) {
      throw Error("Ping command requires 2 values");
    }
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID();
  }
};

} // namespace subsim

#endif // SUBSIM_PING_COMMAND_H
