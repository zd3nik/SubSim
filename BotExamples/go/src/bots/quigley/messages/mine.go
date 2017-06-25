package messages

import (
    "fmt"
    "strconv"
)

type MineHitMessage struct {
    TurnNumber int
    X int
    Y int
    Damage int
}

func MineHit(fields []string) MineHitMessage {
    if (len(fields) != 5) || (fields[0] != "T") {
        panic(fmt.Sprintf("Invalid mine hit message: %v", fields))
    }

    var msg MineHitMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 1) {
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

    msg.Damage, err = strconv.Atoi(fields[4])
    if (err != nil) || (msg.Damage < 1) {
        panic(fmt.Sprintf("Invalid damage: %v", fields))
    }

    return msg
}
