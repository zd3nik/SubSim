package messages

import (
    "fmt"
    "strconv"
)

type SprintActivationsMessage struct {
    TurnNumber int
    Count int
}

func SprintActivations(fields []string) SprintActivationsMessage {
    if (len(fields) != 3) || (fields[0] != "R") {
        panic(fmt.Sprintf("Invalid sprint activations message: %v", fields))
    }

    var msg SprintActivationsMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 0) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    msg.Count, err = strconv.Atoi(fields[2])
    if (err != nil) || (msg.Count < 1) {
        panic(fmt.Sprintf("Invalid count: %v", fields))
    }

    return msg
}
