//-----------------------------------------------------------------------------
// Error.h
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_ERROR_H
#define SUBSIM_ERROR_H

#include "Platform.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Error : public std::runtime_error {
//-----------------------------------------------------------------------------
public: // constructors
  Error() = delete;

  Error(const char* str)
    : std::runtime_error(str ? str : "")
  { }

  Error(const std::string& message)
    : std::runtime_error(message)
  { }

  Error(const Printable& message)
    : std::runtime_error(message.toString())
  { }
};

} // namespace subsim

#endif // SUBSIM_ERROR_H

