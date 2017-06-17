package messages

import (
  "fmt"
  "strconv"
  "strings"
)

type PlayerResultMessage struct {
  Name string
  Score int
}

func PlayerResult(fields []string) PlayerResultMessage {
  if (len(fields) != 3) || (fields[0] != "F") {
    panic(fmt.Sprintf("Invalid player result message: %v", fields))
  }

  var msg PlayerResultMessage
  msg.Name = strings.Trim(fields[1], " \t\v\f\r\n")
  if len(msg.Name) == 0 {
    panic(fmt.Sprintf("Empty player name: %v", fields))
  }

  var err error
  msg.Score, err = strconv.Atoi(fields[2])
  if (err != nil) || (msg.Score < 0) {
    panic(fmt.Sprintf("Invalid score: %v", fields))
  }

  return msg
}
