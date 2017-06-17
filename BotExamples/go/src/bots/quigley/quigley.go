package main

import (
  "os"
  "fmt"
  "flag"
  "math/rand"
  "bots/quigley/utils"
  "bots/quigley/messages"
)

var (
  name = flag.String("name", "Quigley", "The player name to use")
  host = flag.String("host", "localhost", "The game server host address")
  port = flag.Int("port", 9555, "The game server port")
  conn utils.Connection
)

func main() {
  defer errorHandler()

  flag.Parse()
  conn = utils.Connect(*host, *port)

  login()
}

func errorHandler() {
  if err := recover(); err != nil {
    fmt.Fprintf(os.Stderr, "ERROR: %v\n", err)
    os.Exit(1)
  }
}

func login() {
  config := messages.GameConfig(conn.Recv())

  for i := 0; i < config.CustomSettingsCount; i++ {
    val := messages.CustomSetting(conn.Recv())
    // TODO handle custom settings
    fmt.Println(val)
  }

  fmt.Printf("Joining game %q hosted at %q with player name %q\n",
      config.GameTitle, *host, *name)

  x := 1 + rand.Intn(config.MapWidth)
  y := 1 + rand.Intn(config.MapHeight)
  conn.Send(fmt.Sprintf("J|%s|%d|%d", name, x, y))

  response := messages.JoinResponse(conn.Recv())
  if response.Name != *name {
    panic(fmt.Sprintf("Failed to join game: %v", conn.LastRecv))
  }
}
