//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Screen.h"
#include "Error.h"
#include "Logger.h"
#include "Msg.h"
#include "StringUtils.h"
#include <sys/ioctl.h>

namespace subsim
{

//-----------------------------------------------------------------------------
static std::unique_ptr<Screen> instance;

//-----------------------------------------------------------------------------
static Rectangle GetScreenDimensions() {
  struct winsize max;
  if (ioctl(0, TIOCGWINSZ , &max) < 0) {
    throw Error(Msg() << "Failed to get screen dimensions: " << toError(errno));
  }

  Coordinate topLeft(1, 1);
  Coordinate bottomRight(static_cast<unsigned>(max.ws_col),
                         static_cast<unsigned>(max.ws_row));

  if (!bottomRight) {
    throw Error(Msg() << "Invalid screen dimensions: " << bottomRight.getX()
                << 'x' << bottomRight.getY());
  }

  return Rectangle(topLeft, bottomRight);
}

//-----------------------------------------------------------------------------
Screen& Screen::get(const bool update) {
  if (update || !instance) {
    instance.reset(new Screen(GetScreenDimensions()));
  }
  return (*instance);
}

//-----------------------------------------------------------------------------
const char* Screen::colorCode(const ScreenColor color) {
  switch (color) {
  case Red:     return "\033[0;31m";
  case Green:   return "\033[0;32m";
  case Yellow:  return "\033[0;33m";
  case Blue:    return "\033[0;34m";
  case Magenta: return "\033[0;35m";
  case Cyan:    return "\033[0;36m";
  case White:   return "\033[0;37m";
  default:
    break;
  }
  return "\033[0;0m";
}

//-----------------------------------------------------------------------------
Screen& Screen::clear() {
  return str("\033[2J");
}

//-----------------------------------------------------------------------------
Screen& Screen::clearLine() {
  return str("\033[2K");
}

//-----------------------------------------------------------------------------
Screen& Screen::clearToLineBegin() {
  return str("\033[1K");
}

//-----------------------------------------------------------------------------
Screen& Screen::clearToLineEnd() {
  return str("\033[0K");
}

//-----------------------------------------------------------------------------
Screen& Screen::clearToScreenBegin() {
  return str("\033[1J");
}

//-----------------------------------------------------------------------------
Screen& Screen::clearToScreenEnd() {
  return str("\033[0J");
}

//-----------------------------------------------------------------------------
Screen& Screen::color(const ScreenColor color) {
  return str(colorCode(color));
}

//-----------------------------------------------------------------------------
Screen& Screen::cursor(const Coordinate& coord) {
  return cursor(coord.getX(), coord.getY());
}

//-----------------------------------------------------------------------------
Screen& Screen::cursor(const unsigned x, const unsigned y) {
  if (!contains(x, y)) {
    Logger::error() << "invalid screen coordinates: " << x << ',' << y;
    return (*this);
  }
  return str("\033[" + toStr(y) + ';' + toStr(x) + 'H');
}

//-----------------------------------------------------------------------------
Screen& Screen::flag(const ScreenFlag flag) {
  switch (flag) {
  case EL:                 return ch('\n');
  case Flush:              return flush();
  case Clear:              return clear();
  case ClearLine:          return clearLine();
  case ClearToLineBegin:   return clearToLineBegin();
  case ClearToLineEnd:     return clearToLineEnd();
  case ClearToScreenBegin: return clearToScreenBegin();
  case ClearToScreenEnd:   return clearToScreenEnd();
  default:
    break;
  }
  return (*this);
}

//-----------------------------------------------------------------------------
Screen& Screen::flush() {
  if (fflush(stdout)) {
    throw Error(Msg() << "Screen flush failed: " << toError(errno));
  }
  return (*this);
}

//-----------------------------------------------------------------------------
Screen& Screen::str(const std::string& x) {
  if (fwrite(x.c_str(), x.size(), 1, stdout) < 0) {
    throw Error(Msg() << "Failed to print to screen: " << toError(errno));
  }
  return (*this);
}

//-----------------------------------------------------------------------------
Screen& Screen::ch(const char x) {
  if (x && (fputc(x, stdout) != x)) {
    throw Error(Msg() << "Failed to print to screen: " << toError(errno));
  }
  return (*this);
}

} // namespace subsim
