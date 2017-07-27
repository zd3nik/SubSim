package messages

import (
    "fmt"
    "strconv"
    "strings"
)

type SubmarineInfoMessage struct {
    TurnNumber int
    SubId int
    X int
    Y int
    Active bool
    Dead bool
    MineReady bool
    MaxSonarCharge bool
    MaxSprintCharge bool
    MaxTorpedoCharge bool
    Shields int
    Size int
    Torpedos int
    Mines int
    SonarRange int
    SprintRange int
    TorpedoRange int
    SurfaceTurnsRemaining int
    ReactorDamage int
}

func SubmarineInfo(fields []string) SubmarineInfoMessage {
    if (len(fields) < 6) || (fields[0] != "I") {
        panic(fmt.Sprintf("Invalid submarine info message: %v", fields))
    }

    var msg SubmarineInfoMessage
    msg.Size = 100
    msg.Torpedos = -1
    msg.Mines = -1

    var err error
    msg.TurnNumber, err = strconv.Atoi(fields[1])
    if (err != nil) || (msg.TurnNumber < 0) {
        panic(fmt.Sprintf("Invalid turn number: %v", fields))
    }

    msg.SubId, err = strconv.Atoi(fields[2])
    if (err != nil) || (msg.SubId < 0) {
        panic(fmt.Sprintf("Invalid sub ID: %v", fields))
    }

    msg.X, err = strconv.Atoi(fields[3])
    if (err != nil) || (msg.X < 1) {
        panic(fmt.Sprintf("Invalid X coordinate: %v", fields))
    }

    msg.Y, err = strconv.Atoi(fields[4])
    if (err != nil) || (msg.Y < 1) {
        panic(fmt.Sprintf("Invalid Y coordinate: %v", fields))
    }

    val, err := strconv.Atoi(fields[5])
    msg.Active = (val == 1)
    if (err != nil) || ((val != 0) && (val != 1)) {
        panic(fmt.Sprintf("Invalid active flag: %v", fields))
    }

    for _, fld := range fields[6:] {
        flds := strings.Split(strings.Trim(fld, " \t\v\f\r\n"), "=")
        if len(flds) != 2 {
            panic(fmt.Sprintf("Invalid field %q: %v", fld, fields))
        }

        val, err = strconv.Atoi(flds[1])
        if (err != nil) || (val < 0) {
            panic(fmt.Sprintf("Invalid field %q: %v", fld, fields))
        }

        checkBool := false
        switch flds[0] {
        case "shields":
            msg.Shields = val
        case "size":
            msg.Size = val
        case "torpedos":
            msg.Torpedos = val
        case "mines":
            msg.Mines = val
        case "sonar_range":
            msg.SonarRange = val
        case "sprint_range":
            msg.SprintRange = val
        case "torpedo_range":
            msg.TorpedoRange = val
        case "surface_remain":
            msg.SurfaceTurnsRemaining = val
        case "reactor_damage":
            msg.ReactorDamage = val
        case "dead":
            msg.Dead = (val == 1)
            checkBool = true
        case "mine_ready":
            msg.MineReady = (val == 1)
            checkBool = true
        case "max_sonar":
            msg.MaxSonarCharge = (val == 1)
            checkBool = true
        case "max_sprint":
            msg.MaxSprintCharge = (val == 1)
            checkBool = true
        case "max_torpedo":
            msg.MaxTorpedoCharge = (val == 1)
            checkBool = true
        default:
            panic(fmt.Sprintf("Unknown sub-field %q: %v", fld, fields))
        }

        if checkBool && (val > 1) {
            panic(fmt.Sprintf("Invalid field %q: %v", fld, fields))
        }
    }

    return msg
}
