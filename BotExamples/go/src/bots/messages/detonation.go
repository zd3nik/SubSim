package messages

import (
    "fmt"
    "strconv"
)

type DetonationMessage struct {
    TurnNumber int
    X int
    Y int
    Radius int
}

func Detonation(fields []string) DetonationMessage {
    if (len(fields) != 5) || (fields[0] != "D") {
        panic(fmt.Sprintf("Invalid detonation message: %v", fields))
    }

    var msg DetonationMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 0) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    msg.X, err = strconv.Atoi(fields[2])
    if (err != nil) || (msg.X < 1) {
        panic(fmt.Sprintf("Invalid X coordinate: %v", fields))
    }

    msg.Y, err = strconv.Atoi(fields[3])
    if (err != nil) || (msg.Y < 1) {
        panic(fmt.Sprintf("Invalid Y coordinate: %v", fields))
    }

    msg.Radius, err = strconv.Atoi(fields[4])
    if (err != nil) || (msg.Radius < 1) {
        panic(fmt.Sprintf("Invalid radius: %v", fields))
    }

    return msg
}
