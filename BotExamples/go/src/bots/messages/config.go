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
    if len(msg.ServerVersion) == 0 {
        panic(fmt.Sprintf("Empty server version: %v", fields))
    }

    msg.GameTitle = fields[2]
    if len(msg.GameTitle) == 0 {
        panic(fmt.Sprintf("Empty game titel: %v", fields))
    }

    var err error
    msg.MapWidth, err = strconv.Atoi(fields[3])
    if (err != nil) || (msg.MapWidth < 1) {
        panic(fmt.Sprintf("Invalid map width: %v", fields))
    }

    msg.MapHeight, err = strconv.Atoi(fields[4])
    if (err != nil) || (msg.MapHeight < 1) {
        panic(fmt.Sprintf("Invalid map height: %v", fields))
    }

    msg.CustomSettingsCount, err = strconv.Atoi(fields[5])
    if (err != nil) || (msg.CustomSettingsCount < 0) {
        panic(fmt.Sprintf("Invalid custom setting count: %v", fields))
    }

    return msg
}

