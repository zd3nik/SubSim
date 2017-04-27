//-----------------------------------------------------------------------------
// Version.h
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_VERSION_H
#define SUBSIM_VERSION_H

#include "Platform.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Version : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  unsigned parts = 0;
  unsigned numbers = 0;
  unsigned majorNum = 0;
  unsigned minorNum = 0;
  unsigned buildNum = 0;
  std::string other;
  std::string str;

//-----------------------------------------------------------------------------
public: // constructors
  Version() = default;
  Version(const Version&) = default;
  Version(Version&&) noexcept = default;
  Version& operator=(Version&&) noexcept = default;
  Version& operator=(const Version&) = default;

  explicit Version(const std::string& str);

  explicit Version(const unsigned majorNum,
                   const std::string& other = "");

  explicit Version(const unsigned majorNum,
                   const unsigned minorNum,
                   const std::string& other = "");

  explicit Version(const unsigned majorNum,
                   const unsigned minorNum,
                   const unsigned buildNum,
                   const std::string& other = "");

//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override { return str; }

//-----------------------------------------------------------------------------
public: // static constants
  static const Version NO_VERSION;
  static const Version MIN_VERSION;
  static const Version MAX_VERSION;

//-----------------------------------------------------------------------------
public: // methods
  bool isEmpty() const noexcept { return !parts; }
  unsigned partCount() const noexcept { return parts; }
  unsigned numberCount() const noexcept { return numbers; }
  unsigned getMajor() const noexcept { return majorNum; }
  unsigned getMinor() const noexcept { return minorNum; }
  unsigned getBuild() const noexcept { return buildNum; }
  std::string getOther() const { return other; }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return (parts > 0); }
  Version& operator=(const std::string& str);
  bool operator<(const Version& v) const noexcept;
  bool operator>(const Version& v) const noexcept;
  bool operator==(const Version& v) const noexcept;

  bool operator<=(const Version& v) const noexcept {
    return !(operator>(v));
  }

  bool operator>=(const Version& v) const noexcept {
    return !(operator<(v));
  }

  bool operator!=(const Version& v) const noexcept {
    return !(operator==(v));
  }
};

} // namespace subsim

#endif // SUBSIM_VERSION_H

