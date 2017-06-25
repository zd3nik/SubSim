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
    sub messages.SubmarineInfoMessage
    gameMap [][]int
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
    gameMap = make([][]int, config.MapWidth)
    for x := 0; x < config.MapWidth; x++ {
        gameMap[x] = make([]int, config.MapHeight)
        for y := 0; y < config.MapHeight; y++ {
            gameMap[x][y] = 0 // 0 = empty square
        }
    }

    // handle custom settings
    for i := 0; i < config.CustomSettingsCount; i++ {
        val := messages.CustomSetting(conn.Recv())
        switch val.Name {
        case "SubsPerPlayer":
            panic(fmt.Sprintf("This bot only supports 1 sub per player"))
        case "Obstacle":
            x, y := val.Int(0), val.Int(1)
            gameMap[x][y] = BLOCKED
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
    fmt.Println("Game Map Size     :", fmt.Sprintf("%d x %d", config.MapWidth, config.MapHeight))

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

    // clear game map info - it will be re-populated by turn results messages
    for x := 0; x < config.MapWidth; x++ {
        for y := 0; y < config.MapHeight; y++ {
            if gameMap[x][y] != BLOCKED {
                gameMap[x][y] = 0
            }
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
    if gameMap[msg.X][msg.Y] != BLOCKED {
        gameMap[msg.X][msg.Y] = msg.Size
    }
}

func submarineInfo(msg messages.SubmarineInfoMessage) {
    checkTurnNumber(msg.TurnNumber)
    sub = msg
}

func playerScore(msg messages.PlayerScoreMessage) {
    checkTurnNumber(msg.TurnNumber)
    // this bot doesn't do anything with this information
}

func gameFinished(msg messages.GameFinishedMessage) {
    fmt.Println("game finished")
    for i := 0; i < msg.PlayerCount; i++ {
        player := messages.PlayerResult(conn.Recv())
        fmt.Printf("  %s score = %d\n", player.Name, player.Score)
    }
}

func getTorpedoTarget() utils.Coordinate {
    return utils.Coordinate{1,1} // TODO
}

func getDirectionToward(from, to utils.Coordinate) utils.Direction {
    return utils.North // TODO
}

//-------------------------------------------------------------------------
// this is where all the A.I. is
//-------------------------------------------------------------------------
func issueCommand() {
    // always shoot at stuff when we can!
    target := getTorpedoTarget()
    if target.Good() {
        conn.Send(fmt.Sprintf("F|%d|%d|%d|%d", turnNumber, sub.SubId,
            target.X, target.Y))
        randomDestination.Clear()
        return
    }

    // do sonar ping?
    if sub.MaxSonarCharge || ((spotted.Len() == 0) &&
        (sub.TorpedoRange >= sub.SonarRange) &&
        (sub.SonarRange > rand.Intn(20))) {
        conn.Send(fmt.Sprintf("P|%d|%d", turnNumber, sub.SubId))
        randomDestination.Clear()
        return
    }

    // pick an item to charge
    var charge string
    if sub.MaxTorpedoCharge || (sub.Torpedos == 0) || (rand.Intn(100) < 50) {
        charge = "Sonar"
    } else {
        charge = "Torpedo"
    }

    // pick a new random destination square?
    for randomDestination.Bad() || randomDestination.Equals(sub.X, sub.Y) {
        randomDestination = utils.RandomCoordinate(config.MapWidth,
         config.MapHeight)
    }

    // move toward randomDestination
    dir := getDirectionToward(utils.Coordinate{sub.X, sub.Y}, randomDestination)
    conn.Send(fmt.Sprintf("M|%d|%d|%s|%s", turnNumber, sub.SubId,
        dir.Name(), charge))
    lastDirection = dir
}
