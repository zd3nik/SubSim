//-----------------------------------------------------------------------------
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Player.h"
#include "utils/Error.h"
#include "utils/Msg.h"

namespace subsim
{

//-----------------------------------------------------------------------------
Player::Player(const std::string& name, Socket&& tmpSocket)
  : socket(std::move(tmpSocket))
{
  socket.setLabel(name);
}

//-----------------------------------------------------------------------------
std::string
Player::toString() const {
  return replace(socket.toString(), "Socket", "Player");
}

//-----------------------------------------------------------------------------
std::string
Player::summary(const bool gameStarted) const {
  std::stringstream ss;
  ss << handle() << ": " << socket.getLabel();
  if (socket) {
    ss << " [" << socket.getAddress() << ']';
  }
  if (status.size()) {
    ss << " (" << status << ')';
  }
  if (gameStarted) {
    ss << ", Score = " << score << ", Turns = " << turns;
  }
  return ss.str();
}

//-----------------------------------------------------------------------------
void
Player::stealConnectionFrom(Player& other) {
  if (socket) {
    throw Error(Msg() << (*this) << " stealConnectionFrom(" << other
                << ") socket already set");
  }
  const std::string name = socket.getLabel(); // retain current name
  socket = std::move(other.socket);
  socket.setLabel(name);
}

//-----------------------------------------------------------------------------
void
Player::addSubmarine(const Submarine& sub) {
  throw Error("TODO Player::adSubmarine()");
}

//-----------------------------------------------------------------------------
void
Player::addStatsTo(DBRecord& stats, const bool first, const bool last) const {
  const std::string prefix("player." + getName());
  stats.incUInt((prefix + ".total.firstPlace"), (first ? 1 : 0));
  stats.incUInt((prefix + ".total.lastPlace"), (last ? 1 : 0));
  stats.setBool((prefix + ".last.firstPlace"), first);
  stats.setBool((prefix + ".last.lastPlace"), last);
  stats.incUInt((prefix + ".total.score"), score);
  stats.setUInt((prefix + ".last.score"), score);
  stats.incUInt((prefix + ".total.turns"), turns);
  stats.setUInt((prefix + ".last.turns"), turns);
}

//-----------------------------------------------------------------------------
void
Player::saveTo(DBRecord& rec,
               const unsigned opponents,
               const bool first,
               const bool last) const
{
  rec.setString("playerName", getName());
  rec.setString("lastAddress", socket.getAddress());
  rec.setString("lastStatus", status);
  rec.incUInt("gamesPlayed");

  rec.incUInt("total.firstPlace", (first ? 1 : 0));
  rec.incUInt("total.lastPlace", (last ? 1 : 0));
  rec.incUInt("total.opponents", opponents);
  rec.incUInt("total.score", score);
  rec.incUInt("total.turns", turns);

  rec.setBool("last.firstPlace", first);
  rec.setBool("last.lasPlace", last);
  rec.setUInt("last.opponents", opponents);
  rec.setUInt("last.score", score);
  rec.setUInt("last.turns", turns);
}

} // namespace subsim
