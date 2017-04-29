//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_PROCESS_H
#define SUBSIM_PROCESS_H

#include "Platform.h"
#include "Timer.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class Process {
//-----------------------------------------------------------------------------
public: // destructor
  virtual ~Process() noexcept { }

//-----------------------------------------------------------------------------
public: // abstract methods
  virtual bool isRunning() const noexcept = 0;
  virtual bool waitForExit(const Milliseconds timeout = 0) noexcept = 0;
  virtual int getExitStatus() const noexcept = 0;
  virtual int getInputHandle() const = 0;
  virtual void close() noexcept = 0;
  virtual void run() = 0;
  virtual void sendln(const std::string& line) const = 0;
  virtual void validate() const = 0;
  virtual std::string getAlias() const = 0;
};

} // namespace subsim

#endif // SUBSIM_PROCESS_H
