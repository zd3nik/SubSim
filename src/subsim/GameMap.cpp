//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameMap.h"
#include "utils/Error.h"
#include "utils/Movement.h"
#include "utils/Msg.h"

namespace subsim
{

//-----------------------------------------------------------------------------
void
GameMap::reset(const unsigned width, const unsigned height) {
  set(Coordinate(1, 1), Coordinate(width, height));

  squares.clear();
  squares.reserve(getSize());

  for (unsigned y = 1; y <= height; ++y) {
    for (unsigned x = 1; x <= width; ++x) {
      squares.push_back(std::make_unique<Square>(x, y));
#ifndef NDEBUG
      const unsigned idx = (squares.size() - 1);
      const Square& square = (*squares[idx]);
      const unsigned i = toIndex(square);
      ASSERT(i == idx);
      ASSERT(square.getX() == x);
      ASSERT(square.getY() == y);
#endif
    }
  }
  ASSERT(squares.size() == getSize());
}

//-----------------------------------------------------------------------------
void
GameMap::addObject(const Coordinate& coord, ObjectPtr object) {
  Square& square = getSquare(coord);
  if (!square.addObject(object)) {
    throw Error(Msg() << square << " already contains " << (*object));
  }
}

//-----------------------------------------------------------------------------
void
GameMap::removeObject(const Coordinate& coord, ObjectPtr object) {
  Square& square = getSquare(coord);
  if (!square.removeObject(object)) {
    throw Error(Msg() << square << " does not contain " << (*object));
  }
}

//-----------------------------------------------------------------------------
void
GameMap::moveObject(const Coordinate& from,
                    const Coordinate& to,
                    ObjectPtr object)
{
  removeObject(from, object);
  addObject(to, object);
}

//-----------------------------------------------------------------------------
void
GameMap::updateDistances(const unsigned maxDist) {
  maxDistance = maxDist;
  for (auto& square : squares) {
    square->resetDistances();
  }

  for (auto& square : squares) {
    if (square->isOccupied()) {
      square->setDistanceTo((*square), 0);
      updateDistance((*square), (*square), 1);
    }
  }
}

//-----------------------------------------------------------------------------
void
GameMap::updateDistance(Square& origin,
                        const Square& square,
                        const unsigned currentDistance)
{
  if (currentDistance > maxDistance) {
    return;
  }

  std::vector<unsigned> destinations;
  for (Direction dir : { Direction::North,
       Direction::East,
       Direction::South,
       Direction::West })
  {
    const unsigned destIndex = toIndex(square + dir);
    if (destIndex < squares.size()) {
      Square& destination = (*squares[destIndex]);
      const unsigned previousDistance = destination.getDistanceTo(origin);
      if (currentDistance < previousDistance) {
        destination.setDistanceTo(origin, currentDistance);
        origin.setDistanceTo(destination, currentDistance);
        if (destination.isEmpty()) {
          destinations.push_back(destIndex);
        }
      }
    }
  }

  for (unsigned destIndex : destinations) {
    updateDistance(origin, (*squares[destIndex]), (currentDistance + 1));
  }
}

//-----------------------------------------------------------------------------
Square&
GameMap::getSquare(const Coordinate& coord) const {
  const unsigned idx = toIndex(coord);
  if (idx < squares.size()) {
    return (*squares[idx]);
  }
  throw Error(Msg() << "Invalid coordinates: " << coord);
}

} // namespace subsim
