//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_MAP_H
#define SUBSIM_GAME_MAP_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Rectangle.h"
#include "utils/Screen.h"
#include "Object.h"
#include "Square.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameMap : public Rectangle {
//-----------------------------------------------------------------------------
public: // typedefs
  typedef std::pair<std::vector<Coordinate>, std::vector<Coordinate>> TorpedoShot;

//-----------------------------------------------------------------------------
private: // variables
  std::vector<UniqueSquare> squares;

//-----------------------------------------------------------------------------
public: // constructors
  GameMap() = default;
  GameMap(GameMap&&) = delete;
  GameMap(const GameMap&) = delete;
  GameMap& operator=(GameMap&&) = delete;
  GameMap& operator=(const GameMap&) = delete;

//-----------------------------------------------------------------------------
public: // methods
  void animateShot(const Coordinate& mapPos, const TorpedoShot& shot) const;
  void print(Coordinate&) const;
  void printSummary(Coordinate&) const;
  void reset(const unsigned width, const unsigned height);
  void addObject(const Coordinate&, ObjectPtr);
  void removeObject(const Coordinate&, ObjectPtr);
  void moveObject(const Coordinate& from, const Coordinate& to, ObjectPtr);
  bool isBlocked(const Coordinate&) const noexcept;
  Square& getSquare(const Coordinate&) const;

  std::map<Coordinate, unsigned> squaresInRangeOf(
      const Coordinate& coord,
      const unsigned range) const;

//-----------------------------------------------------------------------------
private: // methods
  void printSquare(Screen& screen, const Square& square) const;
  void printRow(Screen& screen, const Coordinate& rowCenter) const;
  void printRow(Screen& screen, const Coordinate& rowCenter,
                const ScreenColor color, const std::string& str) const;
  void addDestinations(std::map<Coordinate, unsigned>& destinations,
                       const Coordinate& to,
                       const unsigned dist,
                       const unsigned range) const;
};

} // namespace subsim

#endif // SUBSIM_GAME_MAP_H
