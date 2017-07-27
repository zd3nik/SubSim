package utils

import (
    "fmt"
    "math/rand"
)

type Coordinate struct {
    X int
    Y int
}

func RandomCoordinate(maxX, maxY int) Coordinate {
    coord := Coordinate{1 + rand.Intn(maxX), 1 + rand.Intn(maxY)}
    for (coord.X > maxX) || (coord.Y > maxY) || (coord.X < 1) || (coord.Y < 1) {
        fmt.Printf("Invalid coordinate generated: %d|%d\n", coord.X, coord.Y)
        coord = Coordinate{1 + rand.Intn(maxX), 1 + rand.Intn(maxY)}
    }
    return coord
}

func (coord *Coordinate) Clear() {
    coord.X = 0
    coord.Y = 0
}

func (coord *Coordinate) Set(x, y int) {
    coord.X = x
    coord.Y = y
}

func (coord *Coordinate) Shifted(dir Direction) Coordinate {
    switch dir {
    case North:
        return Coordinate{coord.X, coord.Y - 1}
    case East:
        return Coordinate{coord.X + 1, coord.Y}
    case South:
        return Coordinate{coord.X, coord.Y + 1}
    case West:
        return Coordinate{coord.X - 1, coord.Y}
    }
    panic(fmt.Sprintf("Illegal direction value: %d", dir))
}

func (coord *Coordinate) Equals(x, y int) bool {
    return (coord.X == x) && (coord.Y == y)
}

func (coord *Coordinate) SameAs(other Coordinate) bool {
    return (coord.X == other.X) && (coord.Y == other.Y)
}

func (coord *Coordinate) Good() bool {
    return (coord.X > 0) && (coord.Y > 0)
}

func (coord *Coordinate) Bad() bool {
    return !coord.Good()
}
