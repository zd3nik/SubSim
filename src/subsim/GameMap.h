//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_GAME_MAP_H
#define SUBSIM_GAME_MAP_H

#include "utils/Platform.h"
#include "utils/Rectangle.h"
#include "Square.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class GameMap : public Rectangle {
//-----------------------------------------------------------------------------
private: // variables
  std::vector<Square> squares;

//-----------------------------------------------------------------------------
public: // constructors
  GameMap() = default;
  GameMap(GameMap&&) = delete;
  GameMap(const GameMap&) = delete;
  GameMap& operator=(GameMap&&) = delete;
  GameMap& operator=(const GameMap&) = delete;

  explicit GameMap(const unsigned width, const unsigned height);
};

} // namespace subsim

#endif // SUBSIM_GAME_MAP_H
