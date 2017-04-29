//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Input.h"
#include "CSVReader.h"
#include "Error.h"
#include "Logger.h"
#include "Msg.h"
#include "StringUtils.h"
#include <sys/select.h>

namespace subsim
{

//-----------------------------------------------------------------------------
char Input::readChar(const int fd) {
  if (fd < 0) {
    throw Error(Msg() << "Input.readChar() invalid handle: " << fd);
  }

  char ch = 0;
  if (read(fd, &ch, 1) != 1) {
    throw Error(Msg() << "Input.readChar() failed: " << toError(errno));
  }

  Logger::debug() << "Received character '" << ch << "' from channel " << fd
                  << " " << getHandleLabel(fd);
  return ch;
}

//-----------------------------------------------------------------------------
ControlKey Input::readKey(const int fd, char& ch) {
  ch = readChar(fd);

  switch (lastChar) {
  case 0:
    switch (ch) {
    case 8:
    case 127:
      return KeyBackspace;
    case 27:
      lastChar = 27;
      return readKey(fd, ch);
    }
    break;
  case 27:
    if (ch == '[') {
      lastChar = '[';
      return readKey(fd, ch);
    }
    break;
  case '3':
    if (ch == '~') {
      lastChar = 0;
      return KeyDel;
    }
    break;
  case '5':
    if (ch == '~') {
      lastChar = 0;
      return KeyPageUp;
    }
    break;
  case '6':
    if (ch == '~') {
      lastChar = 0;
      return KeyPageDown;
    }
    break;
  case '[':
    switch (ch) {
    case 'A':
      lastChar = 0;
      return KeyUp;
    case 'B':
      lastChar = 0;
      return KeyDown;
    case 'H':
      lastChar = 0;
      return KeyHome;
    case 'F':
      lastChar = 0;
      return KeyEnd;
    case '5':
      lastChar = '5';
      return readKey(fd, ch);
    case '6':
      lastChar = '6';
      return readKey(fd, ch);
    }
    break;
  }

  if (lastChar) {
    lastChar = ch = 0;
    return KeyUnknown;
  }

  return KeyChar;
}

//-----------------------------------------------------------------------------
bool Input::waitForData(std::set<int>& ready, const int timeout_ms) {
  ready.clear();
  if (handles.empty()) {
    Logger::warn() << "No input handles specified to wait for";
    return false;
  }

  fd_set set;
  FD_ZERO(&set);

  int maxFd = -1;
  for (auto it = handles.begin(); it != handles.end(); ++it) {
    const int fd = it->first;
    if (fd >= 0) {
      if (buffer.count(fd) && (pos[fd] < len[fd])) {
        ready.insert(fd);
      } else {
        maxFd = std::max<int>(maxFd, fd);
        FD_SET(fd, &set);
      }
    }
  }

  if (ready.empty() && (maxFd >= 0)) {
    struct timeval tv;
    tv.tv_sec = (timeout_ms / 1000);
    tv.tv_usec = ((timeout_ms % 1000) * 10);

    int ret = 0;
    while (true) {
      ret = select((maxFd + 1), &set, nullptr, nullptr,
                   ((timeout_ms < 0) ? nullptr : &tv));
      if (ret < 0) {
        if (errno == EINTR) {
          Logger::debug() << "Input select interrupted";
          return false;
        }
        throw Error(Msg() << "Input select failed: " << toError(errno));
      }
      break;
    }
    if (ret) {
      for (auto it = handles.begin(); it != handles.end(); ++it) {
        const int fd = it->first;
        if (FD_ISSET(fd, &set)) {
          ready.insert(fd);
        }
      }
    }
  }

  return ready.size();
}

//-----------------------------------------------------------------------------
unsigned Input::readln(const int fd, const char delimeter) {
  if (fd < 0) {
    throw Error(Msg() << "Input readln() invalid handle: " << fd);
  }

  line[0] = 0;
  fields.clear();
  if (buffer.find(fd) == buffer.end()) {
    buffer[fd].resize(BUFFER_SIZE, 0);
    pos[fd] = 0;
    len[fd] = 0;
  }

  unsigned n = 0;
  while (n < (BUFFER_SIZE - 1)) {
    if (pos[fd] >= len[fd]) {
      if (!bufferData(fd)) {
        return 0;
      }
    }
    if (!len[fd]) {
      break;
    } else if ((pos[fd] < len[fd]) &&
               ((line[n++] = buffer[fd][pos[fd]++]) == '\n'))
    {
      break;
    }
  }
  line[n] = 0;

  if (Logger::getInstance().getLogLevel() >= Logger::DEBUG) {
    Logger::debug() << "Received '" << trimStr(line.data())
                    << "' from channel " << fd << " " << getHandleLabel(fd);
  }

  unsigned newLineCount = 0;
  for (unsigned i = 0; i < n; ++i) {
    if ((line[0] == '\n') || (line[0] == '\r')) {
      newLineCount++;
    } else {
      break;
    }
  }

  if (!line[0] || (newLineCount == n)) {
    return 0;
  }

  fields = CSVReader(line.data(), delimeter, true).readCells();
  return fields.size();
}

//-----------------------------------------------------------------------------
void Input::addHandle(const int handle, const std::string& label) {
  if (handle >= 0) {
    handles[handle] = label;
    Logger::debug() << "Added channel " << handle << " " << label;
  }
}

//-----------------------------------------------------------------------------
void Input::removeHandle(const int handle) {
  Logger::debug() << "Removing channel " << handle << " "
                  << getHandleLabel(handle);

  auto i1 = handles.find(handle);
  if (i1 != handles.end()) {
    handles.erase(i1);
  }

  auto i2 = buffer.find(handle);
  if (i2 != buffer.end()) {
    buffer.erase(i2);
  }

  auto i3 = pos.find(handle);
  if (i3 != pos.end()) {
    pos.erase(i3);
  }

  auto i4 = len.find(handle);
  if (i4 != len.end()) {
    len.erase(i4);
  }
}

//-----------------------------------------------------------------------------
bool Input::containsHandle(const int handle) const {
  return ((handle >= 0) && (handles.count(handle) > 0));
}

//-----------------------------------------------------------------------------
std::string Input::getHandleLabel(const int handle) const {
  auto it = handles.find(handle);
  if (it != handles.end()) {
    return it->second;
  }
  return std::string();
}

//-----------------------------------------------------------------------------
unsigned Input::getHandleCount() const noexcept {
  return handles.size();
}

//-----------------------------------------------------------------------------
unsigned Input::getFieldCount() const noexcept {
  return fields.size();
}

//-----------------------------------------------------------------------------
std::string Input::getLine(const bool trim) const {
  return trim ? trimStr(line.data()) : std::string(line.data());
}

//-----------------------------------------------------------------------------
std::string Input::getStr(const unsigned index,
                          const std::string& def,
                          const bool trim) const
{
  if (index >= fields.size()) {
    return def;
  } else {
    return trim ? trimStr(fields.at(index)) : fields.at(index);
  }
}

//-----------------------------------------------------------------------------
int Input::getInt(const unsigned index, const int def) const {
  const std::string str = getStr(index);
  return isInt(str) ? toInt32(str) : def;
}

//-----------------------------------------------------------------------------
unsigned Input::getUInt(const unsigned index, const unsigned def) const {
  const std::string str = getStr(index);
  return isUInt(str) ? toUInt32(str) : def;
}

//-----------------------------------------------------------------------------
double Input::getDouble(const unsigned index, const double def) const {
  const std::string str = getStr(index);
  return isFloat(str) ? toDouble(str) : def;
}

//-----------------------------------------------------------------------------
bool Input::bufferData(const int fd) {
  pos[fd] = len[fd] = 0;
  while (len[fd] < BUFFER_SIZE) {
    ssize_t n = read(fd, buffer[fd].data(), BUFFER_SIZE);
    if (n < 0) {
      if (errno == EINTR) {
        Logger::debug() << "Input read interrupted, retrying";
        continue;
      } else {
        Logger::error() << "Input read failed: " << toError(errno);
        return false;
      }
    } else if (n <= BUFFER_SIZE) {
      len[fd] = n;
      break;
    } else {
      throw Error("Input buffer overflow!");
    }
  }
  return true;
}

} // namespace subsim
