package messages

import (
    "fmt"
    "strconv"
)

type PlayerScoreMessage struct {
    TurnNumber int
    Score int
}

func PlayerScore(fields []string) PlayerScoreMessage {
    if (len(fields) != 3) || (fields[0] != "H") {
        panic(fmt.Sprintf("Invalid player score message: %v", fields))
    }

    var msg PlayerScoreMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 1) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    msg.Score, err = strconv.Atoi(fields[2])
    if (err != nil) || (msg.Score < 0) {
        panic(fmt.Sprintf("Invalid score: %v", fields))
    }

    return msg
}
