//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_PRINTABLE_H
#define SUBSIM_PRINTABLE_H

#include "Platform.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Printable {
//-----------------------------------------------------------------------------
public: // destructor
  virtual ~Printable() {}

//-----------------------------------------------------------------------------
public: // abstract methods
  virtual std::string toString() const = 0;

//-----------------------------------------------------------------------------
public: // operator overloads
  operator std::string() const {
    return toString();
  }

  bool operator==(const char* str) const {
    return (toString() == (str ? std::string(str) : std::string()));
  }

  bool operator==(const std::string& str) const {
    return (toString() == str);
  }

  bool operator!=(const char* str) const {
    return (toString() != (str ? std::string(str) : std::string()));
  }

  bool operator!=(const std::string& str) const {
    return (toString() != str);
  }
};

//-----------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const Printable& x) {
  const std::string str = x.toString();
  if (str.size()) {
    os << str;
  }
  return os;
}

//-----------------------------------------------------------------------------
inline std::string& operator+=(std::string& str, const Printable& x) {
  str += x.toString();
  return str;
}

//-----------------------------------------------------------------------------
inline bool operator==(const char* str, const Printable& obj) {
  return (obj == str);
}

//-----------------------------------------------------------------------------
inline bool operator==(const std::string& str, const Printable& obj) {
  return (obj == str);
}

//-----------------------------------------------------------------------------
inline bool operator!=(const char* str, const Printable& obj) {
  return (obj != str);
}

//-----------------------------------------------------------------------------
inline bool operator!=(const std::string& str, const Printable& obj) {
  return (obj != str);
}

} // namespace subsim

#endif // SUBSIM_PRINTABLE_H

