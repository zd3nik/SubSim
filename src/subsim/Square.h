//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SQUARE_H
#define SUBSIM_SQUARE_H

#include "utils/Platform.h"
#include "utils/Coordinate.h"
#include "Object.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Square : public Coordinate {
//-----------------------------------------------------------------------------
public: // variables
  std::list<ObjectPtr> objects;
  std::map<Coordinate, unsigned> distance;

//-----------------------------------------------------------------------------
public: // constructors
  Square() = delete;
  Square(Square&&) = delete;
  Square(const Square&) = delete;
  Square& operator=(Square&&) = delete;
  Square& operator=(const Square&) = delete;

  explicit Square(const unsigned x, const unsigned y) noexcept
    : Coordinate(x, y)
  { }

//-----------------------------------------------------------------------------
public: // methods
  bool isEmpty() const noexcept {
    return objects.empty();
  }

  bool isOccupied() const noexcept {
    return !objects.empty();
  }

  bool isBlocked() const noexcept {
    return ((objects.size() == 1) && objects.front()->isPermanent());
  }

  bool contains(ObjectPtr object) const noexcept {
    Object* ptr = object.get();
    for (ObjectPtr obj : objects) {
      if (obj.get() == ptr) {
        return true;
      }
    }
    return false;
  }

  bool addObject(ObjectPtr object) {
    if (isBlocked()) {
      return false;
    } else if (!contains(object)) {
      objects.push_back(object);
      return true;
    }
    return false;
  }

  bool removeObject(ObjectPtr object) {
    Object* ptr = object.get();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
      if (it->get() == ptr) {
        objects.erase(it);
        return true;
      }
    }
    return false;
  }

  unsigned getSizeOfObjects() const noexcept {
    unsigned size = 0;
    for (const auto& object : objects) {
      if (object->getSize() != ~0U) {
        size += object->getSize();
      } else {
        return ~0U;
      }
    }
    return size;
  }

  unsigned getObjectCount() const noexcept {
    return objects.size();
  }

  std::list<ObjectPtr>::const_iterator begin() const noexcept {
    return objects.begin();
  }

  std::list<ObjectPtr>::const_iterator end() const noexcept {
    return objects.end();
  }

  std::list<ObjectPtr>::iterator begin() noexcept {
    return objects.begin();
  }

  std::list<ObjectPtr>::iterator end() noexcept {
    return objects.end();
  }

  std::list<ObjectPtr>::iterator erase(std::list<ObjectPtr>::iterator it)
  noexcept {
    return objects.erase(it);
  }

  void resetDistances() {
    distance.clear();
  }

  unsigned getDistanceTo(const Coordinate& coord) const {
    auto it = distance.find(coord);
    return (it == distance.end()) ? ~0U : (*it).second;
  }

  void setDistanceTo(const Coordinate& coord, const unsigned dist) {
    distance[coord] = dist;
  }
};

//-----------------------------------------------------------------------------
typedef std::unique_ptr<Square> UniqueSquare;

} // namespace subsim

#endif // SUBSIM_SQUARE_H
