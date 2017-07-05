package messages

import (
    "fmt"
    "strconv"
)

type DiscoveredObjectMessage struct {
    TurnNumber int
    X int
    Y int
    Size int
}

func DiscoveredObject(fields []string) DiscoveredObjectMessage {
    if (len(fields) != 5) || (fields[0] != "O") {
        panic(fmt.Sprintf("Invalid discovered object message: %v", fields))
    }

    var msg DiscoveredObjectMessage
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

    msg.Size, err = strconv.Atoi(fields[4])
    if (err != nil) || (msg.Size < 1) {
        panic(fmt.Sprintf("Invalid size: %v", fields))
    }

    return msg
}
