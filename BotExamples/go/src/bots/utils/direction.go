package utils

import "fmt"

type Direction int

const (
    North Direction = iota
    East
    South
    West
)

func AllDirections() []Direction {
    return []Direction{North, East, South, West}
}

func (dir Direction) Name() string {
    switch dir {
    case North:
        return "N"
    case East:
        return "E"
    case South:
        return "S"
    case West:
        return "W"
    default:
        panic(fmt.Sprintf("Invalid direction value: %d", dir))
    }
}

func (dir Direction) Opposite() Direction {
    switch dir {
    case North:
        return South
    case East:
        return West
    case South:
        return North
    case West:
        return East
    default:
        panic(fmt.Sprintf("Invalid direction value: %d", dir))
    }
}
