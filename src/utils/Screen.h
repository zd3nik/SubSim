//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SCREEN_H
#define SUBSIM_SCREEN_H

#include "Platform.h"
#include "Coordinate.h"
#include "Printable.h"
#include "Rectangle.h"

namespace subsim
{

//-----------------------------------------------------------------------------
enum ScreenColor {
  DefaultColor,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White,
  BrightRed,
  BrightGreen,
  BrightYellow,
  BrightBlue,
  BrightMagenta,
  BrightCyan,
  BrightWhite
};

//-----------------------------------------------------------------------------
enum ScreenFlag {
  EL,
  Flush,
  Clear,
  ClearLine,
  ClearToLineBegin,
  ClearToLineEnd,
  ClearToScreenBegin,
  ClearToScreenEnd
};

//-----------------------------------------------------------------------------
class Screen : public Rectangle {
//-----------------------------------------------------------------------------
private: // constructors
  Screen() = delete;
  Screen(Screen&&) = delete;
  Screen(const Screen&) = delete;
  Screen& operator=(Screen&&) = delete;
  Screen& operator=(const Screen&) = delete;

  explicit Screen(const Rectangle& container)
    : Rectangle(container)
  { }

//-----------------------------------------------------------------------------
public: // static methods
  static Screen& get(const bool update = false);
  static Screen& print() { return get(false); }
  static const char* colorCode(const ScreenColor);

//-----------------------------------------------------------------------------
public: // methods
  Screen& ch(const char);
  Screen& clear();
  Screen& clearLine();
  Screen& clearToLineBegin();
  Screen& clearToLineEnd();
  Screen& clearToScreenBegin();
  Screen& clearToScreenEnd();
  Screen& color(const ScreenColor);
  Screen& cursor(const Coordinate&);
  Screen& cursor(const unsigned x, const unsigned y);
  Screen& flag(const ScreenFlag);
  Screen& flush();
  Screen& str(const std::string&);

  Screen& operator<<(const ScreenColor x) { return color(x); }
  Screen& operator<<(const ScreenFlag x) { return flag(x); }
  Screen& operator<<(const Coordinate& x) { return cursor(x); }
  Screen& operator<<(const Printable& x) { return str(x.toString()); }
  Screen& operator<<(const std::string& x) { return str(x); }
  Screen& operator<<(const char* x) { return str(x ? std::string(x) : ""); }
  Screen& operator<<(const char x) { return ch(x); }

//-----------------------------------------------------------------------------
public: // operator overloads
  Screen& operator<<(const double x) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << x;
    return str(ss.str());
  }

  template<typename T>
  Screen& operator<<(const T& x) {
    return str(toStr(x));
  }
};

} // namespace subsim

#endif // SUBSIM_SCREEN_H
