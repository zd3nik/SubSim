//-----------------------------------------------------------------------------
// GameMap.cpp
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "GameMap.h"

namespace subsim
{

//-----------------------------------------------------------------------------
GameMap::GameMap(const unsigned width, const unsigned height)
  : Rectangle(Coordinate(1, 1), Coordinate(width), height)
{
  squares.resize(GetArea());
}

} // namespace subsim
