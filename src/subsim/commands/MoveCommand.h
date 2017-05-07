//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_MOVE_COMMAND_H
#define SUBSIM_MOVE_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Movement.h"
#include "utils/Msg.h"
#include "Command.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class MoveCommand : public Command {
//-----------------------------------------------------------------------------
public: // constants
  static const char TYPE = 'M';

//-----------------------------------------------------------------------------
private: // variables
  char dirLetter;
  Direction dir;
  Submarine::Equipment equip;

//-----------------------------------------------------------------------------
public: // constructors
  MoveCommand() = delete;
  MoveCommand(MoveCommand&&) noexcept = default;
  MoveCommand(const MoveCommand&) noexcept = default;
  MoveCommand& operator=(MoveCommand&&) noexcept = default;
  MoveCommand& operator=(const MoveCommand&) noexcept = default;

  MoveCommand(const unsigned playerID, const Input& input)
    : Command(Command::MoveTorpedo, playerID, input)
  {
    if (input.getFieldCount() != 5) {
      throw Error("Move command requires 4 values");
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
    equip = Submarine::getEquipment(input.getStr(4));
  }

//-----------------------------------------------------------------------------
public: // Command::Printable implementaion
  std::string toString() const override {
    return Msg(TYPE) << getTurnNumber() << getSubID()
                     << dirLetter << Submarine::equipmentName(equip);
  }
};

} // namespace subsim

#endif // SUBSIM_MOVE_COMMAND_H
