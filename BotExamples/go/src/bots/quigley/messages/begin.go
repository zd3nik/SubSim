package messages

import (
    "fmt"
    "strconv"
)

type BeginTurnMessage struct {
    TurnNumber int
}

func BeginTurn(fields []string) BeginTurnMessage {
    if (len(fields) != 2) || (fields[0] != "B") {
        panic(fmt.Sprintf("Invalid begin turn message: %v", fields))
    }

    var msg BeginTurnMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 1) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    return msg
}

