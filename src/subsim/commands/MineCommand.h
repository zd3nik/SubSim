//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_MINE_COMMAND_H
#define SUBSIM_MINE_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Movement.h"
#include "utils/Msg.h"
#include "Command.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class MineCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'D';

//-----------------------------------------------------------------------------
private: // variables
  char dirLetter;
  Direction dir;

//-----------------------------------------------------------------------------
public: // constructors
  MineCommand() = delete;
  MineCommand(MineCommand&&) noexcept = default;
  MineCommand(const MineCommand&) noexcept = default;
  MineCommand& operator=(MineCommand&&) noexcept = default;
  MineCommand& operator=(const MineCommand&) noexcept = default;

  MineCommand(const unsigned playerID, const Input& input)
    : Command(Command::DeployMine, playerID, input)
  {
    if (input.getFieldCount() != 4) {
      throw Error("Mine command requires 3 values");
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
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID() << dirLetter;
  }
};

} // namespace subsim

#endif // SUBSIM_MINE_COMMAND_H
