//----------------------------------------------------------------------------
// Logger.h
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//----------------------------------------------------------------------------
#ifndef SUBSIM_LOGGER_H
#define SUBSIM_LOGGER_H

#include "Platform.h"
#include "LogStream.h"
#include <fstream>

namespace subsim
{

//-----------------------------------------------------------------------------
class Logger {
//-----------------------------------------------------------------------------
public: // enums
  enum LogLevel {
    ERROR,
    WARN,
    INFO,
    DEBUG
  };

//-----------------------------------------------------------------------------
private: // variables
  LogLevel logLevel = INFO;
  std::ostream* stream = nullptr;
  std::ofstream fileStream;
  std::string logFile;

//-----------------------------------------------------------------------------
private: // constructors
  Logger();
  Logger(Logger&&) = delete;
  Logger(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;

//-----------------------------------------------------------------------------
public: // destructor
  ~Logger();

//-----------------------------------------------------------------------------
public: // static methods
  static Logger& getInstance();

  static inline LogStream printError() {
    return error(true);
  }

  static inline LogStream error(const bool printToStdErr = false) {
    return getInstance().log(ERROR, "ERROR: ", printToStdErr);
  }

  static inline LogStream warn() {
    return getInstance().log(WARN, "WARN: ");
  }

  static inline LogStream info() {
    return getInstance().log(INFO, "INFO: ");
  }

  static inline LogStream debug() {
    return getInstance().log(DEBUG, "DEBUG: ");
  }

//-----------------------------------------------------------------------------
public: // methods
  Logger& setLogLevel(const LogLevel level) noexcept;
  Logger& setLogLevel(const std::string& logLevelStr);
  Logger& setLogFile(const std::string& filePath);
  LogLevel getLogLevel() const noexcept { return logLevel; }
  std::string getLogFile() const { return logFile; }

  LogStream log(const std::string& hdr = "") const {
    return LogStream(stream, hdr);
  }

  LogStream log(const LogLevel level,
      const std::string& hdr = nullptr,
      const bool print = false) const
  {
    return (logLevel >= level) ? LogStream(stream, hdr, print) : LogStream();
  }
};

} // namespace subsim

#endif // SUBSIM_LOGGER_H
