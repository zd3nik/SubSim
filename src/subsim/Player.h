//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_BOARD_H
#define SUBSIM_BOARD_H

#include "Platform.h"
#include "Configuration.h"
#include "Rectangle.h"
#include "Ship.h"
#include "TcpSocket.h"
#include "db/DBRecord.h"

namespace subsim
{

//-----------------------------------------------------------------------------
// The Board class is a Rectangle which represents a printable rectangle
// that shows the player name, ship area, coordinate names around the
// ship area, and one blank line bellow the ship area.
//
//     +-----------------------+
//     | * PlayerName          | <- outer rectangle is the Board
//     |   A B C D E F G H I J |    topLeft is absolute screen position
//     | 1 +-----------------+ |    used for printing to screen
//     | 2 |                 | |
//     | 3 |                 | |
//     | 4 |                 | |
//     | 5 | Inner Rectangle | |
//     | 6 |  is Ship Area   | | <- inner rectangle is the Ship Area
//     | 7 |                 | |    topLeft is always (1,1)
//     | 8 |                 | |    used for aiming not for printing
//     | 9 |                 | |
//     |10 +-----------------+ |
//     |                       |
//     +-----------------------+
//
// A ship area descriptor is used to transfer the ship area across the network.
// It is a single-line string containing the flattened ship area.
// Example ship area: +------+
//                    |.X..X.| (row1)
//                    |0X0..0| (row2)
//                    |...0X.| (row3)
//                    +------+
// Example ship area descriptor: .X..X.0X0..0...0X. (row1row2row3)
//-----------------------------------------------------------------------------
class Board : public Rectangle {
//-----------------------------------------------------------------------------
private: // variables
  bool toMove = false;
  unsigned score = 0;
  unsigned skips = 0;
  unsigned turns = 0;
  Rectangle shipArea;
  TcpSocket socket;
  std::string descriptor;
  std::string status;
  std::vector<std::string> hitTaunts;
  std::vector<std::string> missTaunts;

//-----------------------------------------------------------------------------
public: // constructors
  Board() = default;
  Board(Board&&) = delete;
  Board(const Board&) = delete;
  Board& operator=(Board&&) = delete;
  Board& operator=(const Board&) = delete;

  explicit Board(const std::string& name,
                 const unsigned shipAreaWidth,
                 const unsigned shipAreaHeight,
                 TcpSocket&& = TcpSocket());

  explicit Board(const std::string& name,
                 const Configuration& c,
                 TcpSocket&& sock = TcpSocket())
    : Board(name, c.getBoardWidth(), c.getBoardHeight(), std::move(sock))
  { }

//-----------------------------------------------------------------------------
public: // static methods
  static std::string toString(const std::string& descriptor,
                              const unsigned width);

//-----------------------------------------------------------------------------
public: // Rectangle::Printable implementation
  std::string toString() const override;

//-----------------------------------------------------------------------------
public: // methods
  Rectangle getShipArea() const noexcept { return shipArea; }
  std::string getAddress() const { return socket.getAddress(); }
  std::string getDescriptor() const { return descriptor; }
  std::string getName() const { return socket.getLabel(); }
  std::string getStatus() const { return status; }
  std::vector<std::string> getHitTaunts() const { return hitTaunts; }
  std::vector<std::string> getMissTaunts() const { return missTaunts; }
  bool hasHitTaunts() const noexcept { return !hitTaunts.empty(); }
  bool hasMissTaunts() const noexcept { return !missTaunts.empty(); }
  bool isConnected() const noexcept { return socket.isOpen(); }
  bool isToMove() const noexcept { return toMove; }
  bool send(const std::string& msg) const { return socket.send(msg); }
  int handle() const noexcept { return socket.getHandle(); }
  unsigned getScore() const noexcept { return score; }
  unsigned getSkips() const noexcept { return skips; }
  unsigned getTurns() const noexcept { return turns; }
  void disconnect() noexcept { socket.close(); }

  Board& addHitTaunt(const std::string&);
  Board& addMissTaunt(const std::string&);
  Board& clearHitTaunts() noexcept;
  Board& clearMissTaunts() noexcept;
  Board& incScore(const unsigned = 1) noexcept;
  Board& incSkips(const unsigned = 1) noexcept;
  Board& incTurns(const unsigned = 1) noexcept;
  Board& setName(const std::string&);
  Board& setScore(const unsigned) noexcept;
  Board& setSkips(const unsigned) noexcept;
  Board& setStatus(const std::string&);
  Board& setToMove(const bool) noexcept;
  Board& setTurns(const unsigned) noexcept;
  Board& stealConnectionFrom(Board&&);

  std::string maskedDescriptor() const;
  std::string nextHitTaunt() const;
  std::string nextMissTaunt() const;
  std::string summary(const unsigned boardNum, const bool gameStarted) const;
  std::vector<Coordinate> shipAreaCoordinates() const;

  bool addHitsAndMisses(const std::string& descriptor) noexcept;
  bool addRandomShips(const Configuration&, const double minSurfaceArea);
  bool addShip(const Ship&, Coordinate, const Direction);
  bool isDead() const noexcept;
  bool matchesConfig(const Configuration&) const;
  bool onEdge(const unsigned idx) const noexcept;
  bool onEdge(const Coordinate&) const noexcept;
  bool print(const bool masked, const Configuration* = nullptr) const;
  bool removeShip(const Ship&) noexcept;
  bool updateDescriptor(const std::string& newDescriptor);

  char getSquare(const unsigned idx) const noexcept;
  char getSquare(const Coordinate&) const noexcept;
  char setSquare(const unsigned idx, const char newValue) noexcept;
  char setSquare(const Coordinate&, const char newValue) noexcept;
  char shootSquare(const unsigned idx) noexcept;
  char shootSquare(const Coordinate&) noexcept;

  unsigned adjacentFree(const unsigned idx) const noexcept;
  unsigned adjacentFree(const Coordinate&) const noexcept;
  unsigned adjacentHits(const unsigned idx) const noexcept;
  unsigned adjacentHits(const Coordinate&) const noexcept;
  unsigned distToEdge(const unsigned idx, const Direction) const noexcept;
  unsigned distToEdge(Coordinate, const Direction) const noexcept;
  unsigned freeCount(const unsigned idx, const Direction) const noexcept;
  unsigned freeCount(Coordinate, const Direction) const noexcept;
  unsigned splatCount() const noexcept;
  unsigned hitCount() const noexcept;
  unsigned hitCount(const unsigned idx, const Direction) const noexcept;
  unsigned hitCount(Coordinate, const Direction) const noexcept;
  unsigned horizontalHits(const unsigned idx) const noexcept;
  unsigned horizontalHits(const Coordinate&) const noexcept;
  unsigned maxInlineHits(const unsigned idx) const noexcept;
  unsigned maxInlineHits(const Coordinate&) const noexcept;
  unsigned missCount() const noexcept;
  unsigned shipPointCount() const noexcept;
  unsigned surfaceArea(const unsigned minArea = ~0U) const noexcept;
  unsigned verticalHits(const unsigned idx) const noexcept;
  unsigned verticalHits(const Coordinate&) const noexcept;

  void addStatsTo(DBRecord&, const bool first, const bool last) const;
  void saveTo(DBRecord&, const unsigned opponents,
              const bool first, const bool last) const;

  Coordinate getShipCoord(const unsigned index) const noexcept {
    return shipArea.toCoord(index);
  }

  unsigned getShipIndex(const Coordinate& coord) const noexcept {
    return shipArea.toIndex(coord);
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isValid(); }

//-----------------------------------------------------------------------------
private: // methods
  bool isValid() const noexcept {
    return (Rectangle::isValid() &&
            (shipArea.getSize() > 0) &&
            (descriptor.size() == shipArea.getSize()));
  }

  bool placeShip(std::string& desc, const Ship&, Coordinate, const Direction);
  bool placeShips(std::string& desc,
                  const unsigned minSurfaceArea,
                  const std::vector<Coordinate>& coords,
                  const std::vector<Ship>::iterator& shipBegin,
                  const std::vector<Ship>::iterator& shipEnd);
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Board> BoardPtr;

} // namespace subsim

#endif // SUBSIM_BOARD_H
