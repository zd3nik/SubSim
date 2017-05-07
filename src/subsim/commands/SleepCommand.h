//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SLEEP_COMMAND_H
#define SUBSIM_SLEEP_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "Command.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class SleepCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'S';

//-----------------------------------------------------------------------------
private: // variables
  Submarine::Equipment equip1;
  Submarine::Equipment equip2;

//-----------------------------------------------------------------------------
public: // constructors
  SleepCommand() = delete;
  SleepCommand(SleepCommand&&) noexcept = default;
  SleepCommand(const SleepCommand&) noexcept = default;
  SleepCommand& operator=(SleepCommand&&) noexcept = default;
  SleepCommand& operator=(const SleepCommand&) noexcept = default;

  SleepCommand(const unsigned playerID, const Input& input)
    : Command(Command::Sleep, playerID, input)
  {
    if (input.getFieldCount() != 5) {
      throw Error("Sleep command requires 4 values");
    }
    equip1 = Submarine::getEquipment(input.getStr(3));
    equip2 = Submarine::getEquipment(input.getStr(3));
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID();
  }
};

} // namespace subsim

#endif // SUBSIM_SLEEP_COMMAND_H
