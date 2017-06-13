package messages

import (
  "fmt"
  "strconv"
)

type GameConfigMessage struct {
  ServerVersion string
  GameTitle string
  MapWidth int
  MapHeight int
  CustomSettingsCount int
}

func GameConfig(fields []string) GameConfigMessage {
  if (len(fields) != 6) || (fields[0] != "C") {
    panic(fmt.Sprintf("Invalid game configuration message: %v", fields))
  }

  var msg GameConfigMessage
  msg.ServerVersion = fields[1]
  msg.GameTitle = fields[2]

  var err error
  msg.MapWidth, err = strconv.Atoi(fields[3])
  if (err != nil) || (msg.MapWidth < 1) {
    panic(fmt.Sprintf("Invalid map width: %v", fields[3]))
  }

  msg.MapHeight, err = strconv.Atoi(fields[4])
  if (err != nil) || (msg.MapHeight < 1) {
    panic(fmt.Sprintf("Invalid map height: %v", fields[4]))
  }

  msg.CustomSettingsCount, err = strconv.Atoi(fields[5])
  if (err != nil) || (msg.CustomSettingsCount < 0) {
    panic(fmt.Sprintf("Invalid custom setting count: %v", fields[5]))
  }

  return msg
}

