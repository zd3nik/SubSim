package main

import (
    "os"
    "fmt"
    "flag"
    "math"
    "math/rand"
    "container/list"
    "bots/utils"
    "bots/messages"
)

const (
    BLOCKED = math.MinInt32
    MAX_DIST = math.MaxInt32
)

var (
    name = flag.String("name", "Knute", "The player name to use")
    host = flag.String("host", "localhost", "The game server host address")
    port = flag.Int("port", 9555, "The game server port")
    debugMode = flag.Bool("debug", false, "Run in debug mode")
    conn utils.Connection

    config messages.GameConfigMessage
    mySub messages.SubmarineInfoMessage
    gameMap []Square
    spottedMinRange = 100
    spotted = list.New()
    sprints = list.New()
    detonations = list.New()
    torpedoHits = list.New()
    mineHits = list.New()
    randomDestination utils.Coordinate
    lastDirection utils.Direction = -1
    turnNumber = 0
    probReset = false
)

type Square struct {
    coord utils.Coordinate // the X and Y coordinates of this square
    idx int     // the gameMap index of this square
    size int    // total size of objects in this square
    dist int    // distance to randomDestination
    prob int    // probability (sort of) of enemy occupying this square
    probed bool // temp flag used when updating prob value
}

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

func newSquare(i int) Square {
    return Square {
        coord: mapCoordinate(i),
        idx: i,
        size: 0,
        dist: MAX_DIST,
        prob: 0,
        probed: false,
    }
}

func configure() {
    // get game config message
    config = messages.GameConfig(conn.Recv())

    // initialize game state variables
    turnNumber = 0
    gameMap = make([]Square, (config.MapWidth * config.MapHeight))
    for i, _ := range gameMap {
        gameMap[i] = newSquare(i)
    }

    // handle custom settings
    for i := 0; i < config.CustomSettingsCount; i++ {
        val := messages.CustomSetting(conn.Recv())
        switch val.Name {
        case "SubsPerPlayer":
            panic(fmt.Sprintf("This bot only supports 1 sub per player"))
        case "Obstacle":
            x, y := val.Int(0), val.Int(1)
            gameMap[mapIndex(x, y)].size = BLOCKED
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

    coord := randomCoordinate(utils.Coordinate{})
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
            return
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

    // clear size info in game map
    // it will be re-populated by next turn result msg
    for i, _ := range gameMap {
        if gameMap[i].idx != i {
            panic(fmt.Sprintf("Square at %d has index %d", i, gameMap[i].idx))
        }
        if gameMap[i].size > 0 {
            gameMap[i].size = 0
        }
    }

    // update prob values
    for i, _ := range gameMap {
        gameMap[i].probed = false
    }
    for i, _ := range gameMap {
        if gameMap[i].prob > 0 {
            updateProb(gameMap[i].coord)
        }
    }

    // clear other game state variables
    spottedMinRange++
    spotted = list.New()
    sprints = list.New()
    detonations = list.New()
    torpedoHits = list.New()
    mineHits = list.New()
    probReset = false
}

func sonarDiscovery(msg messages.SonarDiscoveryMessage) {
    checkTurnNumber(msg.TurnNumber)
    spotted.PushBack(msg)
    if msg.Range < spottedMinRange {
        spottedMinRange = msg.Range
    }
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
    if !*debugMode {
        fmt.Printf("    Torpedo Hit at %d|%d, damage=%d\n",
                   msg.X, msg.Y, msg.Damage)
    }
    torpedoHits.PushBack(msg)
}

func mineHit(msg messages.MineHitMessage) {
    checkTurnNumber(msg.TurnNumber)
    if !*debugMode {
        fmt.Printf("    Mine Hit at %d|%d, damage=%d\n",
                   msg.X, msg.Y, msg.Damage)
    }
    mineHits.PushBack(msg)
}

func discoveredObject(msg messages.DiscoveredObjectMessage) {
    checkTurnNumber(msg.TurnNumber)
    idx := mapIndex(msg.X, msg.Y)
    if (msg.Size > 0) && (gameMap[idx].size != BLOCKED) {
        if !probReset {
            probReset = true
            for i, _ := range gameMap {
                gameMap[i].prob = 0
            }
        }
        gameMap[idx].size = msg.Size
        gameMap[idx].prob = 1
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

func updateProb(from utils.Coordinate) {
    next := make([]int, 0, 4)
    for _, dir := range utils.AllDirections() {
        to := from.Shifted(dir)
        idx := toMapIndex(to.X, to.Y)
        if (idx >= 0) && !gameMap[idx].probed &&
           (gameMap[idx].size != BLOCKED) {
            if gameMap[idx].prob > 0 {
                next = append(next, idx)
            }
            gameMap[idx].probed = true
            gameMap[idx].prob++
        }
    }
    for _, idx := range next {
        updateProb(gameMap[idx].coord)
    }
}

func updateDistanceTo(dest utils.Coordinate, distance int) {
    next := make([]int, 0, 4)
    for _, dir := range utils.AllDirections() {
        to := dest.Shifted(dir)
        idx := toMapIndex(to.X, to.Y)
        if (idx > 0) && (gameMap[idx].size != BLOCKED) &&
           (distance < gameMap[idx].dist) {
            gameMap[idx].dist = distance
            next = append(next, idx)
        }
    }
    for _, idx := range next {
        updateDistanceTo(gameMap[idx].coord, (distance + 1))
    }
}

func issueCommand() {
    // always shoot at stuff when we can!
    target := getTorpedoTarget()
    if target.Good() {
        if !*debugMode {
            fmt.Println("  Firing torpedo")
        }
        conn.Send(fmt.Sprintf("F|%d|%d|%d|%d", turnNumber, mySub.SubId,
                              target.X, target.Y))
        randomDestination.Clear()
        return
    }

    // do sonar ping?
    mySubCoord := utils.Coordinate{mySub.X, mySub.Y}
    thresh := 10 + rand.Intn(len(gameMap) / 40)
    if spottedMinRange < thresh {
        thresh = spottedMinRange
    } else if prob := gameMap[mapIndex(mySub.X, mySub.Y)].prob; prob < thresh {
        thresh = prob
    }
    if mySub.MaxSonarCharge || ((spotted.Len() == 0) &&
                                (mySub.TorpedoRange >= mySub.SonarRange) &&
                                (mySub.SonarRange >= thresh)) {
        if !*debugMode {
            fmt.Println("Pinging with range of", mySub.SonarRange)
        }
        conn.Send(fmt.Sprintf("P|%d|%d", turnNumber, mySub.SubId))
        randomDestination.Clear()
        return
    }

    // pick an item to charge
    var charge string
    if mySub.MaxTorpedoCharge || (mySub.Torpedos == 0) ||
       (mySub.TorpedoRange > mySub.SonarRange) {
        charge = "Sonar"
    } else {
        charge = "Torpedo"
    }

    // pick a new random destination square?
    for randomDestination.Bad() || randomDestination.SameAs(mySubCoord) {
        for i, _ := range gameMap {
            gameMap[i].dist = MAX_DIST
        }
        randomDestination = randomCoordinate(mySubCoord)
        updateDistanceTo(randomDestination, 1)
    }

    // move toward randomDestination
    dir := getDirection(mySubCoord)
    conn.Send(fmt.Sprintf("M|%d|%d|%s|%s", turnNumber, mySub.SubId,
                          dir.Name(), charge))
    lastDirection = dir
}

func randomCoordinate(mySubCoord utils.Coordinate) utils.Coordinate {
    coord := utils.RandomCoordinate(config.MapWidth, config.MapHeight)
    sqr := gameMap[mapIndex(coord.X, coord.Y)]
    for tries := 0; coord.SameAs(mySubCoord) ||
                    isOnMapEdge(coord) ||
                    (sqr.size == BLOCKED) ||
                    ((sqr.prob > 0) && (sqr.prob < 7)); tries++ {
        coord = utils.RandomCoordinate(config.MapWidth, config.MapHeight)
        if (tries > 100) && !coord.SameAs(mySubCoord) {
            break
        }
    }
    return coord
}

func isOnMapEdge(coord utils.Coordinate) bool {
    return (coord.X < 2) || (coord.Y < 2) ||
           (coord.X > (config.MapWidth - 3)) ||
           (coord.Y > (config.MapHeight - 3))
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
        if (dist > 1) && (gameMap[i].size > size) {
            coord := gameMap[i].coord
            if blastDistance(mySubCoord, coord) > 1 { // check "blast" distance
                size = gameMap[i].size
                target = coord
            }
        }
    }

    // take shot near recent sonar discovery if no definite target?
    if target.Bad() && (rand.Intn(100) > 66) {
        size = 3
        for i, dist := range squares {
            if dist > 1 {
                p := gameMap[i].prob
                if (p > 0) && (p < size) && (rand.Intn(p + 1) == p) {
                    coord := gameMap[i].coord
                    if blastDistance(mySubCoord, coord) > 1 {
                        size = p
                        target = coord
                    }
                }
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
        if (idx >= 0) && (gameMap[idx].size != BLOCKED) {
            previousDist, exists := dests[idx]
            if (distance < previousDist) || (exists == false) {
                dests[idx] = distance
                next = append(next, to)
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

func getDirection(from utils.Coordinate) utils.Direction {
    // call mapIndex to verify input coordinates are valid
    mapIndex(from.X, from.Y)

    // get randomized list of all directions
    dirs := utils.AllDirections()
    for i, last := 0, (len(dirs) - 1); i < last; i++ {
        if rand.Intn(10) < 5 {
            dirs[i], dirs[last] = dirs[last], dirs[i]
        }
    }

    // pick adjacent square with smallest dist value
    distance := MAX_DIST
    for _, dir := range dirs {
        dest := from.Shifted(dir)
        idx := toMapIndex(dest.X, dest.Y)
        if (idx >= 0) && (gameMap[idx].dist < distance) {
            return dir
        }
    }

    // it should not be possible to get here!
    panic(fmt.Sprintf("No legal direction to move from square %d|%d",
                      from.X, from.Y))
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
