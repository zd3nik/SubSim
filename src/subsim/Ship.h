//-----------------------------------------------------------------------------
// Ship.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SHIP_H
#define SUBSIM_SHIP_H

#include "Platform.h"
#include "Printable.h"
#include "StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Ship : public Printable {
//-----------------------------------------------------------------------------
public: // Printable implementaion
  virtual std::string toString() const {
    return (*this) ? (id + toStr(length)) : "";
  }

//-----------------------------------------------------------------------------
public: // enums
  enum {
    MIN_LENGTH = 2,
    MAX_LENGTH = 8,
    NONE = '.',
    MISS = '0',
    MIN_ID = 'A',
    MAX_ID = 'W',
    HIT = 'X'
  };

//-----------------------------------------------------------------------------
public: // static methods
  static bool isValidID(const char id) noexcept {
    return ((id >= MIN_ID) && (id <= MAX_ID));
  }

  static bool isShip(const char id) noexcept {
    return ((id == HIT) || isValidID(toupper(id)));
  }

  static bool isHit(const char id) noexcept {
    return (isShip(id) && !isValidID(id));
  }

  static bool isMiss(const char id) noexcept {
    return (id == MISS);
  }

  static bool isValidLength(const unsigned length) noexcept {
    return ((length >= MIN_LENGTH) && (length <= MAX_LENGTH));
  }

  static char mask(const char id) noexcept {
    return isValidID(id) ? NONE : isValidID(toupper(id)) ? HIT : id;
  }

  static char hit(const char id) noexcept {
    return isValidID(id) ? tolower(id) : id;
  }

  static char unHit(char& id) noexcept {
    if (isHit(id)) {
      return toupper(id);
    }
    return id;
  }

//-----------------------------------------------------------------------------
private: // variables
  char id = 0;
  unsigned length = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Ship() noexcept = default;
  Ship(Ship&&) noexcept = default;
  Ship(const Ship&) noexcept = default;
  Ship& operator=(Ship&&) noexcept = default;
  Ship& operator=(const Ship&) noexcept = default;

  explicit Ship(const char id, const unsigned length) noexcept
    : id(toupper(id)),
      length(length)
  { }

//-----------------------------------------------------------------------------
public: // methods
  char getID() const noexcept {
    return id;
  }

  unsigned getLength() const noexcept {
    return length;
  }

  bool fromString(const std::string& str) {
    id = 0;
    length = 0;
    if ((str.size() > 1) && isValidID(str[0])) {
      const std::string lenStr = str.substr(1);
      if (isUInt(lenStr)) {
        unsigned len = toUInt32(lenStr);
        if (len > 0) {
          id = str[0];
          length = len;
        }
      }
    }
    return static_cast<bool>(*this);
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept {
    return (isValidID(id) && isValidLength(length));
  }

  bool operator==(const Ship& other) const noexcept {
    return ((id == other.id) && (length == other.length));
  }
};

} // namespace subsim

#endif // SUBSIM_SHIP_H
