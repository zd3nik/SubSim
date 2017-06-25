package utils

import "fmt"

type Direction int

const (
    North Direction = iota
    East
    South
    West
)

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
