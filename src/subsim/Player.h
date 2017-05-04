//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_PLAYER_H
#define SUBSIM_PLAYER_H

#include "utils/Platform.h"
#include "utils/Printable.h"
#include "utils/Socket.h"
#include "db/DBRecord.h"
#include "Submarine.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Player : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  Socket socket;
  std::string status;
  std::vector<Submarine> subs;
  unsigned score = 0;
  unsigned turns = 0;

//-----------------------------------------------------------------------------
public: // constructors
  Player() = delete;
  Player(Player&&) = delete;
  Player(const Player&) = delete;
  Player& operator=(Player&&) = delete;
  Player& operator=(const Player&) = delete;

  explicit Player(const std::string& name, Socket&& sock = Socket());

//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override;

//-----------------------------------------------------------------------------
public: // methods
  void disconnect() noexcept {
    socket.close();
    status = "disconnected";
  }

  bool send(const std::string& msg) const {
    return socket.send(msg);
  }

  std::string summary(const bool gameStarted) const;
  void stealConnectionFrom(Player&);
  void addSubmarine(const Submarine&);
  void addStatsTo(DBRecord&, const bool first, const bool last) const;
  void saveTo(DBRecord&, const unsigned opponents,
              const bool first, const bool last) const;

//-----------------------------------------------------------------------------
public: // setters
  void setName(const std::string& value) {
    socket.setLabel(value);
  }

  void setStatus(const std::string& value) {
    status = value;
  }

  void setScore(const unsigned value) noexcept {
    score = value;
  }

  void incScore(const unsigned value = 1) noexcept {
    score += value;
  }

  void incTurns(const unsigned value = 1) noexcept {
    turns += value;
  }

//-----------------------------------------------------------------------------
public: // getters
  std::string getName() const {
    return socket.getLabel();
  }

  std::string getAddress() const {
    return socket.getAddress();
  }

  std::string getStatus() const {
    return status;
  }

  bool isConnected() const noexcept {
    return socket.isOpen();
  }

  int handle() const noexcept {
    return socket.getHandle();
  }

  unsigned getScore() const noexcept {
    return score;
  }

  unsigned getTurns() const noexcept {
    return turns;
  }

  unsigned getSubmarineCount() const noexcept {
    return subs.size();
  }

  const Submarine& getSubmarine(const unsigned subID) const {
    return subs[subID];
  }

  Submarine& getSubmarine(const unsigned subID) {
    return subs[subID];
  }
};

//-----------------------------------------------------------------------------
typedef std::shared_ptr<Player> PlayerPtr;

} // namespace subsim

#endif // SUBSIM_PLAYER_H
