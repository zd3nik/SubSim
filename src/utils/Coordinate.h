//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_COORDINATE_H
#define SUBSIM_COORDINATE_H

#include "Platform.h"
#include "Movement.h"
#include "Printable.h"
#include "StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Coordinate : public Printable {
//-----------------------------------------------------------------------------
public: // Printable implementation
  virtual std::string toString() const {
    return (toStr(x) + '|' + toStr(y));
  }

//-----------------------------------------------------------------------------
private: // variables
  unsigned x = 0;
  unsigned y = 0;
  double score = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Coordinate() noexcept = default;
  Coordinate(Coordinate&&) noexcept = default;
  Coordinate(const Coordinate&) noexcept = default;
  Coordinate& operator=(Coordinate&&) noexcept = default;
  Coordinate& operator=(const Coordinate&) noexcept = default;

  explicit Coordinate(const unsigned x,
                      const unsigned y,
                      const double score = 0) noexcept
    : x(x),
      y(y),
      score(score)
  { }

//-----------------------------------------------------------------------------
public: // methods
  unsigned getX() const noexcept { return x; }
  unsigned getY() const noexcept { return y; }
  unsigned parity() const noexcept { return ((x & 1) == (y & 1)); }
  double getScore() const noexcept { return score; }

  Coordinate& set(const unsigned x,
                  const unsigned y,
                  const double score = 0) noexcept
  {
    this->x = x;
    this->y = y;
    this->score = score;
    return (*this);
  }

  Coordinate& clear() noexcept {
    return set(0, 0, 0);
  }

  Coordinate& set(const Coordinate& other) noexcept {
    return set(other.x, other.y, other.score);
  }

  Coordinate& setX(const unsigned x) noexcept {
    this->x = x;
    return (*this);
  }

  Coordinate& setY(const unsigned y) noexcept {
    this->y = y;
    return (*this);
  }

  Coordinate& setScore(const double score) noexcept {
    this->score = score;
    return (*this);
  }

  Coordinate& north(const unsigned count = 1) noexcept {
    y = ((*this) && (y >= count)) ? (y - count) : 0;
    return (*this);
  }

  Coordinate& east(const unsigned count = 1) noexcept {
    x = ((*this) && ((x + count) >= x)) ? (x + count) : 0;
    return (*this);
  }

  Coordinate& south(const unsigned count = 1) noexcept {
    y = ((*this) && ((y + count) >= y)) ? (y + count) : 0;
    return (*this);
  }

  Coordinate& west(const unsigned count = 1) noexcept {
    x = ((*this) && (x >= count)) ? (x - count) : 0;
    return (*this);
  }

  Coordinate& shift(const Movement& m) noexcept {
    return shift(m.getDirection(), m.getDistance());
  }

  Coordinate& shift(const Direction dir, const unsigned count = 1) noexcept {
    switch (dir) {
      case North: return north(count);
      case East:  return east(count);
      case South: return south(count);
      case West:  return west(count);
    }
    return (*this);
  }

  bool before(const Coordinate& other) const noexcept {
    return ((y < other.y) || ((y == other.y) && (x < other.x)));
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return (x && y); }

  Coordinate operator+(const Direction dir) const noexcept {
    return Coordinate(*this).shift(dir);
  }

  bool operator==(const Coordinate& other) const noexcept {
    return ((x == other.x) && (y == other.y));
  }

  bool operator!=(const Coordinate& other) const noexcept {
    return !operator==(other);
  }
};

} // namespace subsim

#endif // SUBSIM_COORDINATE_H
