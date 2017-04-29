//----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//----------------------------------------------------------------------------
#include "Logger.h"
#include "CommandArgs.h"
#include "StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
static thread_local std::unique_ptr<Logger> instance;

//-----------------------------------------------------------------------------
Logger&
Logger::getInstance() {
  if (!instance) {
    instance.reset(new Logger());
  }
  return (*instance);
}

//-----------------------------------------------------------------------------
Logger::Logger()
  : logLevel(INFO),
    stream(&std::cerr)
{
  const CommandArgs& args = CommandArgs::getInstance();

  std::string level = args.getStrAfter({"-l", "--log-level"});
  if (isEmpty(level)) {
    level = trimStr(getenv("LOG_LEVEL"));
  }
  if (level.size()) {
    setLogLevel(level);
  }

  std::string file = args.getStrAfter({"-f", "--log-file"});
  if (isEmpty(file)) {
    file = trimStr(getenv("LOG_FILE"));
  }
  if (file.empty()) {
    setLogFile(args.getProgramName() + ".log");
  } else {
    setLogFile(file);
  }
}

//-----------------------------------------------------------------------------
Logger::~Logger() {
  if (fileStream.is_open()) {
    fileStream.close();
  }
}

//-----------------------------------------------------------------------------
Logger&
Logger::setLogFile(const std::string& file) {
  stream = &std::cerr;
  logFile.clear();
  try {
    if (fileStream.is_open()) {
      fileStream.close();
    }
    if (file.size()) {
      fileStream.open(file.c_str(), (std::ofstream::out|std::ofstream::app));
      stream = &fileStream;
      logFile = file;
    }
  } catch (const std::exception& e) {
    error() << "Cannot open " << file << ": " << e.what();
  }
  return (*this);
}

//-----------------------------------------------------------------------------
Logger&
Logger::setLogLevel(const LogLevel level) noexcept {
  logLevel = level;
  return (*this);
}

//-----------------------------------------------------------------------------
Logger&
Logger::setLogLevel(const std::string& level) {
  if (iEqual(level, "DEBUG")) {
    return setLogLevel(DEBUG);
  } else if (iEqual(level, "INFO")) {
    return setLogLevel(INFO);
  } else if (iEqual(level, "WARN")) {
    return setLogLevel(INFO);
  } else if (iEqual(level, "ERROR")) {
    return setLogLevel(INFO);
  }
  Logger::error() << "Invalid log level: '" << level << "'";
  return (*this);
}

} // namespace subsim

