package messages

import (
    "fmt"
    "strings"
)

type JoinResponseMessage struct {
    Name string
}

func JoinResponse(fields []string) JoinResponseMessage {
    if (len(fields) != 2) || (fields[0] != "J") {
        panic(fmt.Sprintf("Join response: %v", fields))
    }

    var msg JoinResponseMessage
    msg.Name = strings.Trim(fields[1], " \t\v\f\r\n")
    if len(msg.Name) == 0 {
        panic(fmt.Sprintf("Empty player name: %v", fields))
    }

    return msg
}
