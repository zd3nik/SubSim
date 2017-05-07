//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_COMMAND_H
#define SUBSIM_COMMAND_H

#include "utils/Platform.h"
#include "utils/Error.h"
#include "utils/Input.h"
#include "utils/Msg.h"
#include "utils/Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Command : public Printable {
//-----------------------------------------------------------------------------
public: // enums
  enum CommandType {
    Invalid,
    Sleep,
    Move,
    Sprint,
    DeployMine,
    FireTorpedo,
    Surface,
    Ping
  };

//-----------------------------------------------------------------------------
private: // variables
  CommandType type = Invalid;
  unsigned playerID = ~0U;
  unsigned turnNumber = ~0U;
  unsigned subID = ~0U;

//-----------------------------------------------------------------------------
public: // constructors
  Command() noexcept = default;
  Command(Command&&) noexcept = default;
  Command(const Command&) noexcept = default;
  Command& operator=(Command&&) noexcept = default;
  Command& operator=(const Command&) noexcept = default;

//-----------------------------------------------------------------------------
protected: // constructors
  Command(const CommandType type,
          const unsigned playerID,
          const unsigned turnNumber,
          const unsigned subID)
    : type(type),
      playerID(playerID),
      turnNumber(turnNumber),
      subID(subID)
  { }

  Command(const CommandType type,
          const unsigned playerID,
          const Input& input)
    : type(type),
      playerID(playerID),
      turnNumber(input.getUInt(1, ~0U)),
      subID(input.getUInt(2, ~0U))
  { }

//-----------------------------------------------------------------------------
public: // getters
  CommandType getType() const noexcept {
    return type;
  }

  unsigned getTurnNumber() const noexcept {
    return turnNumber;
  }

  unsigned getPlayerID() const noexcept {
    return playerID;
  }

  unsigned getSubID() const noexcept {
    return subID;
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  bool operator<(const Command& other) const noexcept {
    return (static_cast<int>(type) < static_cast<int>(other.type));
  }
};

//-----------------------------------------------------------------------------
typedef std::unique_ptr<Command> UniqueCommand;

} // namespace subsim

#endif // SUBSIM_COMMAND_H
