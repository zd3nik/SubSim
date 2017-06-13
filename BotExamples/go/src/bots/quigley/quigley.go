package main

import (
  "os"
  "fmt"
  "strconv"
  "math/rand"
  "bots/quigley/utils"
  "bots/quigley/messages"
)

const (
  DEFAULT_USERNAME = "Quigley"
  DEFAULT_SERVER_ADDRESS = "localhost"
  DEFAULT_SERVER_PORT = 9555
)

var (
  name string
  host string
  port int
  conn utils.Connection
)

func main() {
  defer func() {
    if err := recover(); err != nil {
      fmt.Fprintf(os.Stderr, "ERROR: %v\n", err)
      os.Exit(1)
    }
  }()

  name = getStrArg(1, DEFAULT_USERNAME)
  host = getStrArg(2, DEFAULT_SERVER_ADDRESS)
  port = getIntArg(3, DEFAULT_SERVER_PORT)
  conn = utils.Connect(host, port)

  login()
}

func getStrArg(idx int, def string) string {
  if len(os.Args) > idx {
    return os.Args[idx]
  }
  return def
}

func getIntArg(idx int, def int) int {
  if (len(os.Args) > idx) {
    val, err := strconv.Atoi(os.Args[idx])
    if err != nil {
      panic(err)
    }
    return val
  }
  return def
}

func login() {
  config := messages.GameConfig(conn.Recv())

  for i := 0; i < config.CustomSettingsCount; i++ {
    // TODO properly handle custom settings
    val := conn.Recv()
    fmt.Println(val)
    if val[0] != "V" {
      panic("Invalid custom setting value")
    }
  }

  fmt.Printf("Joining game %q hosted at %q with player name %q\n",
      config.GameTitle, host, name)

  x := 1 + rand.Intn(config.MapWidth)
  y := 1 + rand.Intn(config.MapHeight)
  conn.Send(fmt.Sprintf("J|%s|%d|%d", name, x, y))

}

