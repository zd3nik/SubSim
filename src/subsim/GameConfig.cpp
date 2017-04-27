//-----------------------------------------------------------------------------
// GameConfig.cpp
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameConfig.h"
#include "Msg.h"
#include "Screen.h"
#include "Error.h"

namespace subsim
{

//-----------------------------------------------------------------------------
GameConfig GameConfig::getDefaultGameConfig() {
  return GameConfig();
}

//-----------------------------------------------------------------------------
unsigned GameConfig::getShipTotal() const noexcept {
  unsigned total = 0;
  for (const Ship& ship : ships) {
    total += ship.getLength();
  }
  return total;
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::addShip(const Ship& newShip) {
  if (newShip) {
    ships.push_back(newShip);
    maxSurfaceArea = 0;
    for (const Ship& ship : ships) {
      maxSurfaceArea += ((2 * ship.getLength()) + 2);
    }
  }
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::clearShips() noexcept {
  ships.clear();
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::setBoardSize(const unsigned width,
                                           const unsigned height) noexcept
{
  shipArea = Rectangle(Coordinate(1, 1), Coordinate(width, height));
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::setMaxPlayers(const unsigned maxPlayers) noexcept {
  this->maxPlayers = maxPlayers;
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::setMinPlayers(const unsigned minPlayers) noexcept {
  this->minPlayers = minPlayers;
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::setName(const std::string& name) {
  this->name = name;
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::setPointGoal(const unsigned pointGoal) noexcept {
  this->pointGoal = pointGoal;
  return (*this);
}

//-----------------------------------------------------------------------------
GameConfig& GameConfig::load(Input& input,
                                   bool& gameStarted,
                                   unsigned& playersJoined,
                                   Version& serverVersion)
{
  clear();

  unsigned n = 0;
  std::string msg     = input.getLine();
  std::string str     = input.getStr(n++);
  serverVersion       = input.getStr(n++);
  name                = input.getStr(n++);
  std::string started = input.getStr(n++);
  minPlayers          = input.getUInt(n++);
  maxPlayers          = input.getUInt(n++);
  playersJoined       = input.getUInt(n++, ~0U);
  pointGoal           = input.getUInt(n++);
  unsigned width      = input.getUInt(n++);
  unsigned height     = input.getUInt(n++);
  unsigned shipCount  = input.getUInt(n++);

  if (str != "G") {
    throw Error(Msg() << "Expected game info message, got: " << msg);
  } else if (!serverVersion) {
    throw Error(Msg() << "Invalid version in game info message: " << msg);
  } else if (name.empty()) {
    throw Error(Msg() << "Empty title in game info message: " << msg);
  } else if ((started != "Y") && (started != "N")) {
    throw Error(Msg() << "Invalid started flag in game info message: " << msg);
  } else if (minPlayers < 2) {
    throw Error(Msg() << "Invalid min player count in game info message: "
                << msg);
  } else if (maxPlayers < minPlayers) {
    throw Error(Msg() << "Invalid max player count in game info message: "
                << msg);
  } else if (playersJoined > maxPlayers) {
    throw Error(Msg() << "Invalid joined count in game info message: " << msg);
  } else if (pointGoal < 1) {
    throw Error(Msg() << "Invalid point goal in game info message: " << msg);
  } else if (width < 1) {
    throw Error(Msg() << "Invalid board width in game info message: " << msg);
  } else if (height < 1) {
    throw Error(Msg() << "Invalid board height in game info message: " << msg);
  } else if (shipCount < 1) {
    throw Error(Msg() << "Invalid ship count in game info message: " << msg);
  }

  setBoardSize(width, height);

  Ship ship;
  unsigned maxPointGoal = 0;
  for (unsigned i = 0; i < shipCount; ++i) {
    if (!ship.fromString(str = input.getStr(n++))) {
      throw Error(Msg() << "Invalid ship '" << str
                  << "' in game info message: " << msg);
    }
    maxPointGoal += ship.getLength();
    addShip(ship);
  }

  if (getShipCount() != shipCount) {
    throw Error(Msg() << "Ship count mismatch in game info message: " << msg);
  } else if (pointGoal > maxPointGoal) {
    throw Error(Msg() << "Impossible point goal in game info message: " << msg);
  } else if (!isValid()) {
    throw Error(Msg() << "Invalid game info message: " << msg);
  }

  gameStarted = (started == "Y");
  return (*this);
}

//-----------------------------------------------------------------------------
bool GameConfig::isValidShipDescriptor(const std::string& descriptor) const {
  if (!isValid() || (descriptor.size() != shipArea.getSize())) {
    return false;
  }

  std::string desc(descriptor);
  std::map<char, unsigned> length;
  for (unsigned i = 0; i < desc.size(); ++i) {
    if (!getShip(desc, i, length)) {
      return false;
    }
  }

  if (length.size() != ships.size()) {
    return false;
  }

  for (const Ship& ship : ships) {
    auto it = length.find(ship.getID());
    if ((it == length.end()) || (it->second != ship.getLength())) {
      return false;
    } else {
      length.erase(it);
    }
  }

  return length.empty();
}

//-----------------------------------------------------------------------------
Ship GameConfig::getLongestShip() const noexcept {
  Ship longest;
  for (const Ship& ship : ships) {
    if (!longest || (ship.getLength() > longest.getLength())) {
      longest = ship;
    }
  }
  return longest;
}

//-----------------------------------------------------------------------------
Ship GameConfig::getShortestShip() const noexcept {
  Ship shortest;
  for (const Ship& ship : ships) {
    if (!shortest || (ship.getLength() < shortest.getLength())) {
      shortest = ship;
    }
  }
  return shortest;
}

//-----------------------------------------------------------------------------
void GameConfig::clear() noexcept {
  name.clear();
  minPlayers = 0;
  maxPlayers = 0;
  pointGoal = 0;
  maxSurfaceArea = 0;
  shipArea.set(Coordinate(), Coordinate());
  ships.clear();
}

//-----------------------------------------------------------------------------
void GameConfig::loadFrom(const DBRecord& record) {
  clear();

  name = record.getString("name");
  minPlayers = record.getUInt("minPlayers");
  maxPlayers = record.getUInt("maxPlayers");
  pointGoal = record.getUInt("pointGoal");

  unsigned w = record.getUInt("width");
  unsigned h = record.getUInt("height");
  shipArea.set(Coordinate(1, 1), Coordinate(w, h));

  for (std::string str : record.getStrings("ship")) {
    Ship ship;
    if (ship.fromString(str)) {
      ships.push_back(ship);
    } else {
      throw Error(Msg() << "Invalid ship string: [" << str << ']');
    }
  }

  if (!isValid()) {
    throw Error(Msg() << "Invalid configuration in " << record);
  }
}

//-----------------------------------------------------------------------------
void GameConfig::saveTo(DBRecord& record) const {
  record.setString("name", name);
  record.setUInt("minPlayers", minPlayers);
  record.setUInt("maxPlayers", maxPlayers);
  record.setUInt("pointGoal", pointGoal);
  record.setUInt("width", shipArea.getWidth());
  record.setUInt("height", shipArea.getHeight());

  record.clear("ship");
  for (const Ship& ship : ships) {
    record.addString("ship", ship.toString());
  }
}

//-----------------------------------------------------------------------------
void GameConfig::print(Coordinate& coord) const {
  unsigned w = shipArea.getWidth();
  unsigned h = shipArea.getHeight();
  Screen::print() << coord         << "Title       : " << name;
  Screen::print() << coord.south() << "Min Players : " << minPlayers;
  Screen::print() << coord.south() << "Max Players : " << maxPlayers;
  Screen::print() << coord.south() << "Board Size  : " << w << " x " << h;
  Screen::print() << coord.south() << "Point Goal  : " << pointGoal;
  Screen::print() << coord.south() << "Ship Count  : " << ships.size();

  coord.south();

  for (const Ship& ship : ships) {
    Screen::print() << coord.south().setX(3)
                    << ship.getID() << ": length " << ship.getLength();
  }

  Screen::print() << coord.south().setX(1);
}

//-----------------------------------------------------------------------------
bool GameConfig::isValid() const noexcept {
  return (name.size() && ships.size() && shipArea &&
          (minPlayers > 1) && (maxPlayers >= minPlayers) &&
          (shipArea.getWidth() <= (Ship::MAX_ID - Ship::MIN_ID)) &&
          (shipArea.getHeight() <= (Ship::MAX_ID - Ship::MIN_ID)) &&
          ((pointGoal + maxSurfaceArea) <= shipArea.getSize()));
}

//-----------------------------------------------------------------------------
bool GameConfig::getShip(std::string& desc,
                            const unsigned startIndex,
                            std::map<char, unsigned>& length) const
{
  if (startIndex >= desc.size()) {
    ASSERT(false);
    return false; // error!
  }

  const char id = desc[startIndex];
  desc[startIndex] = '*';

  if ((id == '*') || (id == Ship::NONE) || (id == Ship::MISS)) {
    return true; // ignore previously visited squares, empty squares, misses
  } else if (!Ship::isValidID(id) || length.count(id)) {
    return false; // invalid or duplicate ship id
  }

  unsigned i;
  length[id] = 1;
  Coordinate coord(shipArea.toCoord(startIndex));
  if (shipArea.contains(coord.east())) {
    while (((i = shipArea.toIndex(coord)) < desc.size()) && (desc[i] == id)) {
      desc[i] = '*';
      length[id]++;
      coord.east();
    }
  }

  if (length[id] == 1) {
    coord.set(shipArea.toCoord(startIndex));
    if (shipArea.contains(coord.south())) {
      while (((i = shipArea.toIndex(coord)) < desc.size()) && (desc[i] == id)) {
        desc[i] = '*';
        length[id]++;
        coord.south();
      }
    }
  }

  return (length[id] > 1);
}

} // namespace subsim
