package main

import (
    "os"
    "fmt"
    "flag"
    "math"
    "math/rand"
    "container/list"
    "bots/quigley/utils"
    "bots/quigley/messages"
)

const (
    BLOCKED = math.MinInt32
)

var (
    name = flag.String("name", "Quigley", "The player name to use")
    host = flag.String("host", "localhost", "The game server host address")
    port = flag.Int("port", 9555, "The game server port")
    debugMode = flag.Bool("debug", false, "Run in debug mode")
    conn utils.Connection

    config messages.GameConfigMessage
    mySub messages.SubmarineInfoMessage
    gameMap []int
    spotted = list.New()
    sprints = list.New()
    detonations = list.New()
    torpedoHits = list.New()
    mineHits = list.New()
    randomDestination utils.Coordinate
    lastDirection utils.Direction = -1
    turnNumber = 0
)

func main() {
    defer errorHandler()

    flag.Parse()
    utils.Debug = (utils.Debug || *debugMode)
    conn = utils.Connect(*host, *port)

    configure()
    login()
    play()
}

func errorHandler() {
    if err := recover(); err != nil {
        fmt.Fprintf(os.Stderr, "ERROR: %v\n", err)
        os.Exit(1)
    }
}

func configure() {
    // get game config message
    config = messages.GameConfig(conn.Recv())

    // initialize game state variables
    turnNumber = 0
    gameMap = make([]int, (config.MapWidth * config.MapHeight))
    for i, _ := range gameMap {
        gameMap[i] = 0 // 0 = empty square
    }

    // handle custom settings
    for i := 0; i < config.CustomSettingsCount; i++ {
        val := messages.CustomSetting(conn.Recv())
        switch val.Name {
        case "SubsPerPlayer":
            panic(fmt.Sprintf("This bot only supports 1 sub per player"))
        case "Obstacle":
            x, y := val.Int(0), val.Int(1)
            gameMap[mapIndex(x, y)] = BLOCKED
        default:
            fmt.Println("CustomSetting     :", val)
        }
    }
}

func login() {
    fmt.Println("Joining as Player :", *name)
    fmt.Println("Server Host:Port  :", fmt.Sprintf("%s:%d", *host, *port))
    fmt.Println("Server Version    :", config.ServerVersion)
    fmt.Println("Game Title        :", config.GameTitle)
    fmt.Println("Game Map Size     :", fmt.Sprintf("%d x %d",
                                                   config.MapWidth,
                                                   config.MapHeight))

    coord := utils.RandomCoordinate(config.MapWidth, config.MapHeight)
    conn.Send(fmt.Sprintf("J|%s|%d|%d", *name, coord.X, coord.Y))

    response := messages.JoinResponse(conn.Recv())
    if response.Name != *name {
        panic(fmt.Sprintf("Failed to join game: %v", conn.LastRecv))
    }
}

func play() {
    for {
        fields := conn.Recv()
        switch fields[0] {
        case "B":
            beginTurn(messages.BeginTurn(fields))
        case "S":
            sonarDiscovery(messages.SonarDiscovery(fields))
        case "R":
            sprintActivations(messages.SprintActivations(fields))
        case "D":
            detonation(messages.Detonation(fields))
        case "T":
            torpedoHit(messages.TorpedoHit(fields))
        case "M":
            mineHit(messages.MineHit(fields))
        case "O":
            discoveredObject(messages.DiscoveredObject(fields))
        case "I":
            submarineInfo(messages.SubmarineInfo(fields))
        case "H":
            playerScore(messages.PlayerScore(fields))
        case "F":
            gameFinished(messages.GameFinished(fields))
            break // exit the loop
        default:
            panic(fmt.Sprintf("Unknown message type: %v", fields))
        }
    }
}

func checkTurnNumber(n int) {
    if n != turnNumber {
        panic(fmt.Sprintf("Expected turn number %d, got %d: %q",
            turnNumber, n, conn.LastRecv))
    }
}

func beginTurn(msg messages.BeginTurnMessage) {
    checkTurnNumber(msg.TurnNumber - 1)

    // issue submarine command for this turn
    turnNumber = msg.TurnNumber
    issueCommand();

    // clear game map info - it will be re-populated by next turn result msg
    for i, val := range gameMap {
        if val != BLOCKED {
            gameMap[i] = 0
        }
    }

    // clear other game state variables
    spotted = list.New()
    sprints = list.New()
    detonations = list.New()
    torpedoHits = list.New()
    mineHits = list.New()
}

func sonarDiscovery(msg messages.SonarDiscoveryMessage) {
    checkTurnNumber(msg.TurnNumber)
    spotted.PushBack(msg)
}

func sprintActivations(msg messages.SprintActivationsMessage) {
    checkTurnNumber(msg.TurnNumber)
    sprints.PushBack(msg)
}

func detonation(msg messages.DetonationMessage) {
    checkTurnNumber(msg.TurnNumber)
    detonations.PushBack(msg)
}

func torpedoHit(msg messages.TorpedoHitMessage) {
    checkTurnNumber(msg.TurnNumber)
    torpedoHits.PushBack(msg)
}

func mineHit(msg messages.MineHitMessage) {
    checkTurnNumber(msg.TurnNumber)
    mineHits.PushBack(msg)
}

func discoveredObject(msg messages.DiscoveredObjectMessage) {
    checkTurnNumber(msg.TurnNumber)
    idx := mapIndex(msg.X, msg.Y)
    if (msg.Size >= 100) && (gameMap[idx] != BLOCKED) {
        gameMap[idx] = msg.Size
    }
}

func submarineInfo(msg messages.SubmarineInfoMessage) {
    checkTurnNumber(msg.TurnNumber)
    mySub = msg
}

func playerScore(msg messages.PlayerScoreMessage) {
    checkTurnNumber(msg.TurnNumber)
    // this bot doesn't do anything with this information
}

func gameFinished(msg messages.GameFinishedMessage) {
    fmt.Println("game", msg.Status)
    for i := 0; i < msg.PlayerCount; i++ {
        player := messages.PlayerResult(conn.Recv())
        fmt.Printf("  %s score = %d\n", player.Name, player.Score)
    }
}

func issueCommand() {
    // always shoot at stuff when we can!
    target := getTorpedoTarget()
    if target.Good() {
        conn.Send(fmt.Sprintf("F|%d|%d|%d|%d", turnNumber, mySub.SubId,
                              target.X, target.Y))
        randomDestination.Clear()
        return
    }

    // do sonar ping?
    if mySub.MaxSonarCharge || ((spotted.Len() == 0) &&
                                (mySub.TorpedoRange >= mySub.SonarRange) &&
                                (mySub.SonarRange > 10 + rand.Intn(10))) {
        conn.Send(fmt.Sprintf("P|%d|%d", turnNumber, mySub.SubId))
        randomDestination.Clear()
        return
    }

    // pick an item to charge
    var charge string
    if mySub.MaxTorpedoCharge || (mySub.Torpedos == 0) || (rand.Intn(10) < 4) {
        charge = "Sonar"
    } else {
        charge = "Torpedo"
    }

    // pick a new random destination square?
    // TODO prefer destination to be away from enemies and center of map
    mySubCoord := utils.Coordinate{mySub.X, mySub.Y}
    for randomDestination.Bad() || randomDestination.SameAs(mySubCoord) ||
        isCentral(randomDestination) || isOnMapEdge(randomDestination) {
        randomDestination = utils.RandomCoordinate(config.MapWidth,
                                                   config.MapHeight)
    }

    // move toward randomDestination
    dir := getDirectionToward(mySubCoord, randomDestination)
    conn.Send(fmt.Sprintf("M|%d|%d|%s|%s", turnNumber, mySub.SubId,
                          dir.Name(), charge))
    lastDirection = dir
}

func isCentral(coord utils.Coordinate) bool {
    x := math.Abs((float64(config.MapWidth) / 2) - float64(coord.X))
    y := math.Abs((float64(config.MapHeight) / 2) - float64(coord.Y))
    xMin := float64(config.MapWidth) / 4
    yMin := float64(config.MapHeight) / 4
    result := (x <= xMin) && (y <= yMin)
    if *debugMode && result {
        fmt.Printf("Central coordinate: %d x %d\n", coord.X, coord.Y)
    }
    return result
}

func isOnMapEdge(coord utils.Coordinate) bool {
    result := (coord.X < 2) || (coord.Y < 2) ||
              (coord.X > (config.MapWidth - 3)) ||
              (coord.Y > (config.MapHeight - 3))
    if *debugMode && result {
        fmt.Printf("Edge coordinate: %d x %d\n", coord.X, coord.Y)
    }
    return result
}

func getTorpedoTarget() utils.Coordinate {
    // initialize target to invalid coordinate
    target := utils.Coordinate{}

    // exit immediately if insufficient torpedo charge
    if mySub.TorpedoRange < 2 {
        return target
    }

    // get map of squares within torpedo range
    mySubCoord := utils.Coordinate{mySub.X, mySub.Y}
    squares := squaresInRangeOf(mySubCoord, mySub.TorpedoRange)

    // set target to an enemy occupied square within torpedo range
    size := 0 // pick the square with the largest occupied size
    for i, dist := range squares { // dist = "range" distance
        if (dist > 1) && (gameMap[i] > size) {
            coord := mapCoordinate(i)
            if blastDistance(mySubCoord, coord) > 1 { // check "blast" distance
                size = gameMap[i]
                target = coord
            }
        }
    }

    return target
}

func squaresInRangeOf(from utils.Coordinate, maxRange int) map[int]int {
    i := mapIndex(from.X, from.Y)
    dests := make(map[int]int) // [mapIndex]dist
    dests[i] = 0
    if maxRange > 0 {
        addDestinations(dests, from, 1, maxRange)
    }
    return dests
}

func addDestinations(dests map[int]int, from utils.Coordinate,
                     distance int, maxRange int) {
    next := make([]utils.Coordinate, 0, 4)
    for _, dir := range utils.AllDirections() {
        to := from.Shifted(dir)
        idx := toMapIndex(to.X, to.Y)
        if (idx >= 0) && (gameMap[idx] != BLOCKED) {
            previousDist, exists := dests[idx]
            if (distance < previousDist) || (exists == false) {
                dests[idx] = distance
                if gameMap[idx] <= 0 {
                    next = append(next, to)
                }
            }
        }
    }

    if distance < maxRange {
        for _, coord := range next {
            addDestinations(dests, coord, distance + 1, maxRange)
        }
    }
}

func blastDistance(from, to utils.Coordinate) int {
    // call mapIndex to verify input coordinates are valid
    mapIndex(from.X, from.Y)
    mapIndex(to.X, to.Y)

    // now calculate blast distance
    xDiff := math.Abs(float64(from.X - to.X))
    yDiff := math.Abs(float64(from.Y - to.Y))
    return int(math.Max(xDiff, yDiff))
}

func getDirectionToward(from, to utils.Coordinate) utils.Direction {
    if from.SameAs(to) {
        panic("getDirectionToward() coordinates are the same")
    }

    // call mapIndex to verify input coordinates are valid
    mapIndex(from.X, from.Y)
    mapIndex(to.X, to.Y)

    // get list of all directions
    dirs := utils.AllDirections()

    // move intended direction(s) to front of directions list
    n := 0
    if to.X < from.X {
        n = swapElements(dirs, n, utils.West)
    } else if to.X > from.X {
        n = swapElements(dirs, n, utils.East)
    }
    if to.Y < from.Y {
        n = swapElements(dirs, n, utils.North)
    } else if to.Y > from.Y {
        n = swapElements(dirs, n, utils.South)
    }

    // randomize if more than one intended direction
    if (n > 1) && (rand.Intn(10) < 5) {
        dirs[0], dirs[1] = dirs[1], dirs[0]
    }

    // randomize remaining directions
    for last := len(dirs) - 1; n < last; n++ {
        if rand.Intn(10) < 5 {
            dirs[n], dirs[last] = dirs[last], dirs[n]
        }
    }

    // return first legal direction from the list
    var altDir utils.Direction = -1
    for _, dir := range dirs {
        coord := from.Shifted(dir)
        if coord.Good() && (dir.Opposite() != lastDirection) &&
           (gameMap[mapIndex(coord.X, coord.Y)] != BLOCKED) {
            if isCentral(coord) {
                if altDir == -1 {
                    altDir = dir
                }
            } else {
                return dir
            }
        }
    }

    if altDir != -1 {
        return altDir
    }

    // it should not be possible to get here!
    panic(fmt.Sprintf("No legal direction to move from square %d|%d",
                      from.X, from.Y))
}

func swapElements(dirs []utils.Direction, idx int, dir utils.Direction) int {
    for i := idx; i < len(dirs); i++ {
        if dirs[i] == dir {
            if i != idx {
                dirs[idx], dirs[i] = dirs[i], dirs[idx]
            }
            return idx + 1
        }
    }
    return idx
}

func mapIndex(x, y int) int {
    i := toMapIndex(x, y)
    if i < 0 {
        panic(fmt.Sprintf("Illegal map coordinates: %d, %d", x, y))
    }
    return i
}

func toMapIndex(x, y int) int {
    if (x < 1) || (x > config.MapWidth) || (y < 1) || (y > config.MapHeight) {
        return -1
    }
    return (x - 1) + ((y - 1) * config.MapWidth)
}

func mapCoordinate(i int) utils.Coordinate {
    coord := toMapCoordinate(i)
    if coord.Bad() {
        panic(fmt.Sprintf("Illegal map index: %d", i))
    }
    return coord
}

func toMapCoordinate(i int) utils.Coordinate {
    if (i < 0) || (i >= len(gameMap)) {
        return utils.Coordinate{}
    }
    x := (i % config.MapWidth) + 1
    y := (i / config.MapWidth) + 1
    return utils.Coordinate{x, y}
}
