//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_LOGSTREAM_H
#define SUBSIM_LOGSTREAM_H

#include "Platform.h"
#include <iostream>

namespace subsim
{

//-----------------------------------------------------------------------------
class LogStream {
//-----------------------------------------------------------------------------
private: // variables
  std::ostream* stream = nullptr;
  bool print = false;

//-----------------------------------------------------------------------------
public: // constructors
  LogStream() noexcept = default;
  LogStream(LogStream&&) noexcept = default;
  LogStream(const LogStream&) = delete;
  LogStream& operator=(LogStream&&) noexcept = default;
  LogStream& operator=(const LogStream&) = delete;

  explicit LogStream(std::ostream* stream,
                     const std::string& hdr = "",
                     const bool print = false)
    : stream(stream),
      print(print && (stream != &(std::cerr)))
  {
    if (hdr.size()) {
      if (stream) {
        (*stream) << hdr;
      }
      if (print) {
        std::cerr << hdr;
      }
    }
  }

//-----------------------------------------------------------------------------
public: // destructor
  ~LogStream() {
    if (stream) {
      (*stream) << std::endl;
      stream->flush();
    }
    if (print) {
      std::cerr << std::endl;
    }
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  LogStream& operator<<(const char* x) {
    if (x && (*x)) {
      if (stream) {
        (*stream) << x;
      }
      if (print) {
        std::cerr << x;
      }
    }
    return (*this);
  }

  template<class T>
  LogStream& operator<<(const T& x) {
    if (stream) {
      (*stream) << x;
    }
    if (print) {
      std::cerr << x;
    }
    return (*this);
  }
};

} // namespace subsim

#endif // SUBSIM_LOGSTREAM_H

