//-----------------------------------------------------------------------------
// Input.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_INPUT_H
#define SUBSIM_INPUT_H

#include "Platform.h"

namespace subsim
{

//-----------------------------------------------------------------------------
enum ControlKey : char {
  KeyChar,
  KeyUp,
  KeyDown,
  KeyHome,
  KeyEnd,
  KeyPageUp,
  KeyPageDown,
  KeyDel,
  KeyBackspace,
  KeyUnknown
};

//-----------------------------------------------------------------------------
class Input {
//-----------------------------------------------------------------------------
public: // enums
  enum {
    BUFFER_SIZE = 4096
  };

//-----------------------------------------------------------------------------
private: // variables
  char lastChar = 0;
  std::vector<char> line;
  std::vector<std::string> fields;
  std::map<int, std::string> handles;
  std::map<int, std::vector<char>> buffer;
  std::map<int, unsigned> pos;
  std::map<int, unsigned> len;

//-----------------------------------------------------------------------------
public: // constructors
  Input() : line(BUFFER_SIZE, 0) { }
  Input(Input&&) = delete;
  Input(const Input&) = delete;
  Input& operator=(Input&&) = delete;
  Input& operator=(const Input&) = delete;

//-----------------------------------------------------------------------------
public: // methods
  /**
   * @brief Read a single character from the given handle
   *
   * This method does not alter internal buffers or field state,
   * it does a direct read on the given handle
   *
   * @param handle The handle to read data from
   * @return the first available char read from the given handle
   */
  char readChar(const int handlde);

  /**
   * @brief Read the next complete key sequence from the given handle
   *
   * This method does not alter internal buffers of field state,
   * it does direct reads on the given handle to determine the next key
   * sequence
   *
   * @param handle The handle to read data from
   * @param ch Update with char read from given handle if return type = KeyChar
   * @return the type of key sequence read from the given handle
   */
  ControlKey readKey(const int handle, char& ch);

  /**
   * @brief Block execution until timeout or data becomes available for reading
   *
   * Wait for data to become available for reading on one or more of the
   * handles added via this::addHandle()
   *
   * @param[out] ready Populated with handles that have data available
   * @param timeout_ms max milliseconds to wait, -1 = wait indefinitely
   * @return true if data available, otherwise false
   */
  bool waitForData(std::set<int>& ready, const int timeout_ms = -1);

  /**
   * @brief Read one line of data from the given handle
   *
   * Read data from given handle up to whichever of these comes first:
   *
   *  * the first new-line character
   *  * (BUFFER_SIZE - 1) bytes
   *  * no more data available
   *
   * Then split the data into fields using the specified delimiter.
   * You can the get individual field values via:
   *
   *  * this::getStr(fieldIndex)
   *  * this::getInt(fieldIndex)
   *  * this::getUInt(fieldIndex)
   *  * this::getDouble(fieldIndex)
   *
   * @param handle The handle to read data from
   * @param delimeter If not 0 split line into fields via this delimeter
   * @return number of '|' delimited fields read
   */
  unsigned readln(const int handle, const char delimeter = '|');

  void addHandle(const int handle, const std::string& label = "");
  void removeHandle(const int handle);
  bool containsHandle(const int handle) const;
  unsigned getHandleCount() const noexcept;
  unsigned getFieldCount() const noexcept;

  int       getInt(const unsigned index = 0, const int def = -1) const;
  unsigned getUInt(const unsigned index = 0, const unsigned def = 0) const;
  double getDouble(const unsigned index = 0, const double def = 0) const;

  std::string getHandleLabel(const int handle) const;
  std::string getLine(const bool trim = true) const;
  std::string getStr(const unsigned index = 0,
                     const std::string& def = "",
                     const bool trim = true) const;

//-----------------------------------------------------------------------------
private: // methods
  bool bufferData(const int fd);
};

} // namespace subsim

#endif // SUBSIM_INPUT_H
