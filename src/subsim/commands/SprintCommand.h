//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SPRINT_COMMAND_H
#define SUBSIM_SPRINT_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Movement.h"
#include "utils/Msg.h"
#include "Command.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class SprintCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'R';

//-----------------------------------------------------------------------------
private: // variables
  char dirLetter;
  unsigned dist;
  Direction dir;

//-----------------------------------------------------------------------------
public: // constructors
  SprintCommand() = delete;
  SprintCommand(SprintCommand&&) noexcept = default;
  SprintCommand(const SprintCommand&) noexcept = default;
  SprintCommand& operator=(SprintCommand&&) noexcept = default;
  SprintCommand& operator=(const SprintCommand&) noexcept = default;

  SprintCommand(const unsigned playerID, const Input& input)
    : Command(Command::SprintTorpedo, playerID, input)
  {
    if (input.getFieldCount() != 5) {
      throw Error("Sprint command requires 4 values");
    }
    const std::string str = input.getStr(3);
    if (str.size() != 1) {
      throw Error("Invalid direction: " + str);
    }
    switch (dirLetter = str[0]) {
    case 'E':
      dir = Direction::East;
      break;
    case 'N':
      dir = Direction::North;
      break;
    case 'S':
      dir = Direction::South;
      break;
    case 'W':
      dir = Direction::West;
      break;
    default:
      throw Error("Invalid direction: " + str);
    }
    if ((dist = input.getUInt(3)) < 2) {
      throw Error("Invalid distance: " + input.getStr(3));
    }
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID() << dirLetter << dist;
  }
};

} // namespace subsim

#endif // SUBSIM_SPRINT_COMMAND_H
