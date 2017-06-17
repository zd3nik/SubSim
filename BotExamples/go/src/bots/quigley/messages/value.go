package messages

import (
  "fmt"
  "strconv"
  "strings"
)

type CustomSettingMessage struct {
  Name string
  Values []string
}

func CustomSetting(fields []string) CustomSettingMessage {
  if (len(fields) < 3) || (fields[0] != "V") {
    panic(fmt.Sprintf("Invalid custom setting message: %v", fields))
  }

  var msg CustomSettingMessage
  msg.Name = strings.Trim(fields[1], " \t\v\f\r\n")
  if len(msg.Name) == 0 {
    panic(fmt.Sprintf("Empty setting name: %v", fields))
  }

  msg.Values = fields[1:]
  return msg
}

func (msg *CustomSettingMessage) IntVal() int {
  return msg.Int(0)
}

func (msg *CustomSettingMessage) Int(idx int) int {
  if idx >= len(msg.Values) {
    panic(fmt.Sprintf("msg %s has %d values: %v", msg.Name,
          len(msg.Values), msg.Values))
  }

  val, err := strconv.Atoi(msg.Values[idx])
  if (err != nil) || (val < 0) {
    panic(fmt.Sprintf("Invalid %s value: %v", msg.Name, msg.Values[idx]))
  }

  return val
}
