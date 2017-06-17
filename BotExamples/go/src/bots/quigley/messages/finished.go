package messages

import (
  "fmt"
  "strconv"
  "strings"
)

type GameFinishedMessage struct {
  PlayerCount int
  TurnCount int
  Status string
}

func GameFinished(fields []string) GameFinishedMessage {
  if (len(fields) != 4) || (fields[0] != "F") {
    panic(fmt.Sprintf("Invalid game finished message: %v", fields))
  }

  var msg GameFinishedMessage
  var err error
  msg.PlayerCount, err = strconv.Atoi(fields[1])
  if (err != nil) || (msg.PlayerCount < 1) {
    panic(fmt.Sprintf("Invalid player count: %v", fields))
  }

  msg.TurnCount, err = strconv.Atoi(fields[2])
  if (err != nil) || (msg.TurnCount < 0) {
    panic(fmt.Sprintf("Invalid turn count: %v", fields))
  }

  msg.Status = strings.Trim(fields[3], " \t\v\f\r\n")
  return msg
}
