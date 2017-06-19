package utils

import (
  "math/rand"
)

type Coordinate struct {
  X int
  Y int
}

func (coord *Coordinate) Clear() {
  coord.X = 0
  coord.Y = 0
}

func (coord *Coordinate) Set(x, y int) {
  coord.X = x
  coord.Y = y
}

func (coord *Coordinate) Equals(x, y int) bool {
  return (coord.X == x) && (coord.Y == y)
}

func (coord *Coordinate) Good() bool {
  return (coord.X > 0) || (coord.Y > 0)
}

func (coord *Coordinate) Bad() bool {
  return !coord.Good()
}

func RandomCoordinate(maxX, maxY int) Coordinate {
  return Coordinate{1 + rand.Intn(maxX), 1 + rand.Intn(maxY)}
}
