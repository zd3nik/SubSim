package main

import (
  "os"
  "fmt"
  "strconv"
  "bots/utils"
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
  fields := conn.Recv()
  fmt.Println(fields)
  if (fields[0] != "C") || (len(fields) != 6) {
    panic("Invalid game configuration message")
  }

  count, err := strconv.Atoi(fields[5])
  if (err != nil) || (count < 0) {
    panic("Invalid custom setting count")
  }

  for i := 0; i < count; i++ {
    val := conn.Recv()
    fmt.Println(val)
    if val[0] != "V" {
      panic("Invalid custom setting value")
    }
  }

  x, y := 1, 1 // TODO use random start coordinates
  conn.Send(fmt.Sprintf("J|%s|%d|%d", name, x, y))

}

