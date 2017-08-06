//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameMap.h"
#include "Submarine.h"
#include "utils/Error.h"
#include "utils/Movement.h"
#include "utils/Msg.h"

namespace subsim
{

//-----------------------------------------------------------------------------
void
GameMap::printSquare(Screen& screen, const Square& square) const {
  const unsigned count = square.getObjectCount();
  if (count > 1) {
    ASSERT(!square.isBlocked());
    screen << rPad(count, 3, ' ');
  } else if (count) {
    const Object* obj = square.begin()->get();
    const Submarine* sub = dynamic_cast<const Submarine*>(obj);
    const char ch = obj->getMapChar();
    const std::string str = rPad(toStr(ch), 3, ' ');
    if (sub) {
      switch (sub->getShieldCount()) {
      case 0:
        screen << Red << str << DefaultColor;
        break;
      case 1:
        screen << Yellow << str << DefaultColor;
        break;
      case 2:
        screen << Green << str << DefaultColor;
        break;
      default:
        screen << White << str << DefaultColor;
      }
    } else {
      screen << str;
    }
  } else {
    screen << "  .";
  }
}

//-----------------------------------------------------------------------------
static Coordinate toScreenCoord(Coordinate coord) {
  return Coordinate(((coord.getX() * 3) + 1), (coord.getY() + 1));
}

//-----------------------------------------------------------------------------
void
GameMap::animateShot(const Coordinate& mapPos, const TorpedoShot& shot) const {
  ASSERT(shot.first.size());
  ASSERT(shot.second.size());
  Screen& screen = Screen::print() << mapPos;
  for (const Coordinate& coord : shot.first) {
    screen << toScreenCoord(coord) << Blue << "  *" << mapPos << Flush;
    usleep(100000);
    screen << toScreenCoord(coord) << DefaultColor;
    printSquare(screen, getSquare(coord));
  }

  unsigned count = 0;
  const Coordinate& finalDest = shot.first.back();
  for (unsigned dist = 0; count < shot.second.size(); ++dist) {
    for (const Coordinate& coord : shot.second) {
      if (finalDest.blastDistanceTo(coord) == dist) {
        screen << toScreenCoord(coord) << Red << "  X";
        count++;
      }
    }
    screen << mapPos << Flush;
    usleep(250000);
  }
  usleep(250000);

  for (Coordinate coord : shot.second) {
    screen << toScreenCoord(coord) << DefaultColor;
    printSquare(screen, getSquare(coord));
  }
  screen.flush();
}

//-----------------------------------------------------------------------------
void
GameMap::print(Coordinate& coord) const {
  Screen& screen = Screen::print() << coord << "   ";
  for (unsigned x = 1; x <= getWidth(); ++x) {
    screen << rPad(x, 3, ' ');
  }
  for (unsigned y = 1; y <= getHeight(); ++y) {
    screen << coord.south() << rPad(y, 3, ' ');
    for (unsigned x = 1; x <= getWidth(); ++x) {
      printSquare(screen, getSquare(Coordinate(x, y)));
    }
  }
  screen << coord.south(2);
}

//-----------------------------------------------------------------------------
void
GameMap::printSummary(Coordinate& coord) const {
  unsigned count = 0;
  for (const UniqueSquare& square : squares) {
    count += square->getObjectCount();
  }

  Screen& screen = Screen::print() << coord << "Object Count: " << count;
  if (count) {
    unsigned tmp = 0;
    for (const UniqueSquare& square : squares) {
      for (const ObjectPtr& object : (*square)) {
        if (!object->isPermanent()) {
          if (!tmp++) {
            screen << coord.south().setX(4);
          }
          screen << coord.south() << object->toString();
        }
      }
    }
    if (tmp) {
      screen << coord.south(2).setX(1);
    }
  }
}

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
  if (square.isBlocked()) {
    throw Error(Msg() << square << " is blocked, can't add objects to it");
  } else if (!square.addObject(object)) {
    throw Error(Msg() << square << " already contains " << (*object));
  }
  object->setLocation(coord);
}

//-----------------------------------------------------------------------------
void
GameMap::removeObject(const Coordinate& coord, ObjectPtr object) {
  Square& square = getSquare(coord);
  if (!square.removeObject(object)) {
    throw Error(Msg() << square << " does not contain " << (*object));
  }
  object->setLocation(Coordinate());
}

//-----------------------------------------------------------------------------
void
GameMap::moveObject(const Coordinate& from,
                    const Coordinate& to,
                    ObjectPtr object)
{
  Square& fromSquare = getSquare(from);
  Square& toSquare = getSquare(to);
  if (!object) {
    throw Error("GameMap::moveObject() null object");
  } else if (toSquare.isBlocked()) {
    throw Error(Msg() << toSquare << " is blocked, can't add objects to it");
  } else if (!fromSquare.removeObject(object)) {
    throw Error(Msg() << fromSquare << " does not contain " << (*object));
  } else if (!toSquare.addObject(object)) {
    throw Error(Msg() << toSquare << " already contains " << (*object));
  }
  object->setLocation(toSquare);
}

//-----------------------------------------------------------------------------
std::map<Coordinate, unsigned>
GameMap::squaresInRangeOf(const Coordinate& coord,
                          const unsigned range) const
{
  if (!contains(coord)) {
    throw Error(Msg() << "Invalid coordinates: " << coord);
  }
  std::map<Coordinate, unsigned> destinations;
  destinations[coord] = 0;
  if (range > 0) {
    addDestinations(destinations, coord, 1, range);
  }
  return std::move(destinations);
}

//-----------------------------------------------------------------------------
void
GameMap::addDestinations(std::map<Coordinate, unsigned>& dests,
                         const Coordinate& coord,
                         const unsigned distance,
                         const unsigned range) const
{
  ASSERT(distance <= range);
  std::vector<unsigned> next;
  for (Direction dir : {
       Direction::North,
       Direction::East,
       Direction::South,
       Direction::West })
  {
    const unsigned destIndex = toIndex(coord + dir);
    if (destIndex < squares.size()) {
      Square& dest = (*squares[destIndex]);
      const auto it = dests.find(dest);
      const unsigned previousDist = (it == dests.end()) ? ~0U : it->second;
      if (distance < previousDist) {
        dests[dest] = distance;
        if (dest.isEmpty()) {
          next.push_back(destIndex);
        }
      }
    }
  }

  if (distance < range) {
    for (unsigned idx : next) {
      addDestinations(dests, (*squares[idx]), (distance + 1), range);
    }
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
