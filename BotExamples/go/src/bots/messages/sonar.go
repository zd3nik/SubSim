package messages

import (
    "fmt"
    "strconv"
)

type SonarDiscoveryMessage struct {
    TurnNumber int
    SubId int
    Range int
}

func SonarDiscovery(fields []string) SonarDiscoveryMessage {
    if (len(fields) != 4) || (fields[0] != "S") {
        panic(fmt.Sprintf("Invalid sonar discovery message: %v", fields))
    }

    var msg SonarDiscoveryMessage
    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 0) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    msg.SubId, err = strconv.Atoi(fields[2])
    if (err != nil) || (msg.SubId < 0) {
        panic(fmt.Sprintf("Invalid sub ID: %v", fields))
    }

    msg.Range, err = strconv.Atoi(fields[3])
    if (err != nil) || (msg.Range < 0) {
        panic(fmt.Sprintf("Invalid range: %v", fields))
    }

    return msg
}

