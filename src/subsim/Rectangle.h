//-----------------------------------------------------------------------------
// Rectangle.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_RECTANGLE_H
#define SUBSIM_RECTANGLE_H

#include "Platform.h"
#include "Coordinate.h"
#include "Movement.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Rectangle : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  Coordinate begin;
  Coordinate end;
  unsigned width = 0;
  unsigned height = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Rectangle() noexcept = default;
  Rectangle(Rectangle&&) noexcept = default;
  Rectangle(const Rectangle&) noexcept = default;
  Rectangle& operator=(Rectangle&&) noexcept = default;
  Rectangle& operator=(const Rectangle&) noexcept = default;

  explicit Rectangle(const Coordinate& topLeft,
                     const Coordinate& bottomRight) noexcept
    : begin(topLeft),
      end(bottomRight),
      width((begin && end) ? (end.getX() - begin.getX() + 1) : 0),
      height((begin && end) ? (end.getY() - begin.getY() + 1) : 0)
  { }

//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override {
    return (toStr(getWidth()) + 'x' + toStr(getHeight()) + " Rectangle");
  }

//-----------------------------------------------------------------------------
public: // methods
  Rectangle& set(const Coordinate& topLeft,
                 const Coordinate& bottomRight) noexcept
  {
    begin = topLeft;
    end = bottomRight;
    width = (begin && end) ? (end.getX() - begin.getX() + 1) : 0;
    height = (begin && end) ? (end.getY() - begin.getY() + 1) : 0;
    return (*this);
  }

  Coordinate getTopLeft() const noexcept { return begin; }
  Coordinate getBottomRight() const noexcept { return end; }
  unsigned getMinX() const noexcept { return begin.getX(); }
  unsigned getMaxX() const noexcept { return end.getX(); }
  unsigned getMinY() const noexcept { return begin.getY(); }
  unsigned getMaxY() const noexcept { return end.getY(); }
  unsigned getWidth() const noexcept { return width; }
  unsigned getHeight() const noexcept { return height; }
  unsigned getSize() const noexcept { return (width * height); }

  Coordinate toCoord(const unsigned index) const noexcept;
  unsigned toIndex(const Coordinate&) const noexcept;
  bool arrangeChildren(std::vector<Rectangle*>& children) const noexcept;

  Rectangle& shift(const Direction dir, const unsigned count) noexcept {
    return set(begin.shift(dir, count), end.shift(dir, count));
  }

  bool contains(const unsigned x, const unsigned y) const noexcept {
    return (isValid() &&
            (x >= getMinX()) && (x <= getMaxX()) &&
            (y >= getMinY()) && (y <= getMaxY()));
  }

  bool contains(const Coordinate& coord) const noexcept {
    return (coord && contains(coord.getX(), coord.getY()));
  }

  bool contains(const Rectangle& other) const noexcept {
    return (other && contains(other.begin) && contains(other.end));
  }

  bool moveCoordinate(Coordinate& coord, const Movement& m) const noexcept {
    return (contains(coord) && contains(coord.shift(m)));
  }

  bool moveCoordinate(Coordinate& coord,
                      const Direction dir,
                      const unsigned distance) const noexcept
  {
    return (contains(coord) && contains(coord.shift(dir, distance)));
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isValid();  }

  bool operator<(const Rectangle& other) const noexcept {
    return begin.before(other.begin);
  }

  bool operator==(const Rectangle& other) const noexcept {
    return ((begin == other.begin) && (end == other.end));
  }

  bool operator!=(const Rectangle& other) const noexcept {
    return ((begin != other.begin) || (end != other.end));
  }

//-----------------------------------------------------------------------------
protected: // methods
  bool isValid() const noexcept {
    return (begin && end &&
            (getMinX() <= getMaxX()) &&
            (getMinY() <= getMaxY()));
  }
};

} // namespace subsim

#endif // SUBSIM_RECTANGLE_H
