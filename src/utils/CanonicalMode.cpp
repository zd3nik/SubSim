//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "CanonicalMode.h"
#include "Error.h"
#include "Logger.h"
#include "Msg.h"
#include "StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
CanonicalMode::CanonicalMode(const bool enabled)
  : ok(false)
{
  termios ios;
  if (tcgetattr(STDIN_FILENO, &ios) < 0) {
    throw Error(Msg() << "tcgetattr failed: " << toError(errno));
  }

  savedTermIOs = ios;
  if (enabled) {
    ios.c_lflag |= (ICANON | ECHO);
  } else {
    ios.c_lflag &= ~(ICANON | ECHO);
  }

  if (tcsetattr(STDIN_FILENO, TCSANOW, &ios) < 0) {
    throw Error(Msg() << "tcsetattr failed: " << toError(errno));
  }

  ok = true;
}

//-----------------------------------------------------------------------------
CanonicalMode::~CanonicalMode() noexcept {
  if (ok) {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &savedTermIOs) < 0) {
      try {
        Logger::error() << "failed to restore termios: " << toError(errno);
      } catch (...) {
        ASSERT(false);
      }
    }
  }
}

} // namespace subsim
