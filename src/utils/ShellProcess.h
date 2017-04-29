//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_SHELL_PROCESS_H
#define SUBSIM_SHELL_PROCESS_H

#include "Platform.h"
#include "Pipe.h"
#include "Process.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class ShellProcess : public Process {
//-----------------------------------------------------------------------------
public: // enums
  enum IOType {
    INPUT_ONLY,
    OUTPUT_ONLY,
    BIDIRECTIONAL
  };

//-----------------------------------------------------------------------------
private: // variables
  IOType ioType;
  std::string alias;
  std::string shellCommand;
  std::string shellExecutable;
  std::vector<std::string> commandArgs;
  int childPid = -1;
  int exitStatus = -1;
  Pipe inPipe;
  Pipe outPipe;
  Pipe errPipe; // TODO add interface(s) to use errPipe

//-----------------------------------------------------------------------------
public: // constructors
  explicit ShellProcess(const IOType,
                        const std::string& alias,
                        const std::string& shellCommand);

  explicit ShellProcess(const IOType,
                        const std::string& alias,
                        const std::string& shellExecutable,
                        const std::vector<std::string> args);

  explicit ShellProcess(const std::string& alias,
                        const std::string& shellCommand)
    : ShellProcess(BIDIRECTIONAL, alias, shellCommand)
  { }

  explicit ShellProcess(const std::string& alias,
                        const std::string& shellExecutable,
                        const std::vector<std::string> args)
    : ShellProcess(BIDIRECTIONAL, alias, shellExecutable, args)
  { }

  ShellProcess(ShellProcess&&) noexcept = default;
  ShellProcess(const ShellProcess&) = delete;
  ShellProcess& operator=(ShellProcess&&) noexcept = default;
  ShellProcess& operator=(const ShellProcess&) = delete;

//-----------------------------------------------------------------------------
public: // destructor
  virtual ~ShellProcess() noexcept { close(); }

//-----------------------------------------------------------------------------
public: // static methods
  static std::string joinStr(const std::vector<std::string>&);
  static std::vector<std::string> splitStr(const std::string&);

//-----------------------------------------------------------------------------
public: // Process implementation
  bool isRunning() const noexcept override;
  bool waitForExit(const Milliseconds timeout = 0) noexcept override;
  int getExitStatus() const noexcept override { return exitStatus; }
  int getInputHandle() const override;
  void close() noexcept override;
  void run() override;
  void sendln(const std::string& line) const override;
  void validate() const override;
  std::string getAlias() const override { return alias; }

//-----------------------------------------------------------------------------
public: // methods
  int getChildPID() const noexcept { return childPid; }
  IOType getIOType() const noexcept { return ioType; }
  std::string getShellCommand() const { return shellCommand; }
  std::vector<std::string> getCommandArgs() const { return commandArgs; }
  std::string readln(const Milliseconds timeout = 0) const;

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return (childPid > 0); }

//-----------------------------------------------------------------------------
private: // methods
  std::string readln(const int fd, const Milliseconds timeout) const;
  void runChild();
  void runParent();
};

} // namespace subsim

#endif // SUBSIM_SHELL_PROCESS_H
