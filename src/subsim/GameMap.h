//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_MAP_H
#define SUBSIM_GAME_MAP_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "utils/Rectangle.h"
#include "Object.h"
#include "Square.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameMap : public Rectangle {
//-----------------------------------------------------------------------------
private: // variables
  std::vector<UniqueSquare> squares;
  unsigned maxDistance;

//-----------------------------------------------------------------------------
public: // constructors
  GameMap() = default;
  GameMap(GameMap&&) = delete;
  GameMap(const GameMap&) = delete;
  GameMap& operator=(GameMap&&) = delete;
  GameMap& operator=(const GameMap&) = delete;

//-----------------------------------------------------------------------------
public: // methods
  void print(Coordinate&) const;
  void printSummary(Coordinate&) const;
  void reset(const unsigned width, const unsigned height);
  void addObject(const Coordinate&, ObjectPtr);
  void removeObject(const Coordinate&, ObjectPtr);
  void moveObject(const Coordinate& from, const Coordinate& to, ObjectPtr);
  void updateDistances(const unsigned maxDistance);
  bool isBlocked(const Coordinate&) const noexcept;
  Square& getSquare(const Coordinate&) const;

//-----------------------------------------------------------------------------
private: // methods
  void updateDistance(Square& from, const Square& to, const unsigned dist);
};

} // namespace subsim

#endif // SUBSIM_GAME_MAP_H
