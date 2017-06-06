package utils

import (
  "fmt"
  "net"
  "bufio"
  "strings"
)

type Connection struct {
  reader *bufio.Reader
  writer *bufio.Writer
}

func Connect(host string, port int) Connection {
  tcp, err := net.Dial("tcp", fmt.Sprintf("%s:%d", host, port))
  if err != nil {
    panic(err)
  }

  var conn Connection = Connection {
    reader: bufio.NewReader(tcp),
    writer: bufio.NewWriter(tcp),
  }

  return conn
}

func (conn Connection) Send(message string) {
  _, err := conn.writer.WriteString(message + "\n")
  if err != nil {
    panic(err)
  }
  err = conn.writer.Flush()
  if err != nil {
    panic(err)
  }
}

func (conn Connection) Recv() []string {
  message, err := conn.reader.ReadString('\n')
  if err != nil {
    panic(err)
  }

  trimmed := strings.Trim(message, " \t\v\f\r\n")
  if len(trimmed) == 0 {
    panic("utils.Recv() no data from server")
  }

  fields := strings.Split(trimmed, "|")
  if len(fields[0]) != 1 {
    panic("utils.Recv() ERROR: " + trimmed)
  }

  return fields
}

