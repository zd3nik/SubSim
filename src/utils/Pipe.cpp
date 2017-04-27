//-----------------------------------------------------------------------------
// Pipe.h
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "Pipe.h"
#include "Msg.h"
#include "StringUtils.h"
#include "Error.h"
#include <csignal>
#include <cstring>
#include <fcntl.h>

namespace subsim {

//-----------------------------------------------------------------------------
Pipe Pipe::SELF_PIPE;

//-----------------------------------------------------------------------------
static void selfPipeSignal(int sigNumber) {
  const int eno = errno;
  Pipe::SELF_PIPE.writeln(toStr(sigNumber));
  errno = eno;
}

//-----------------------------------------------------------------------------
void Pipe::openSelfPipe() {
  // TODO make thread safe
  if (!SELF_PIPE.canRead() && !SELF_PIPE.canWrite()) {
    SELF_PIPE.open();

    int fr = SELF_PIPE.fdRead;
    int fw = SELF_PIPE.fdWrite;
    if ((fcntl(fr, F_SETFL, fcntl(fr, F_GETFL) | O_NONBLOCK) < 0) ||
        (fcntl(fw, F_SETFL, fcntl(fw, F_GETFL) | O_NONBLOCK) < 0))
    {
      throw Error(Msg() << "Pipe.openSelfPipe() fcntl failed: "
                  << toError(errno));
    }

    signal(SIGCHLD, selfPipeSignal);
    signal(SIGALRM, selfPipeSignal);
    signal(SIGUSR1, selfPipeSignal);
    signal(SIGUSR2, selfPipeSignal);
  }
}

//-----------------------------------------------------------------------------
void Pipe::open() {
  if ((fdRead >= 0) || (fdWrite >= 0)) {
    throw Error("Pipe.open() already open");
  }

  int fd[] = { -1, -1 };
  if (::pipe(fd) < 0) {
    throw Error(Msg() << "Pipe.open() failed: " << toError(errno));
  }

  fdRead = fd[0];
  fdWrite = fd[1];
}

//-----------------------------------------------------------------------------
void Pipe::close() noexcept {
  closeRead();
  closeWrite();
}

//-----------------------------------------------------------------------------
void Pipe::closeRead() noexcept {
  if (fdRead >= 0) {
    ::close(fdRead);
    fdRead = -1;
  }
}

//-----------------------------------------------------------------------------
void Pipe::closeWrite() noexcept {
  if (fdWrite >= 0) {
    ::close(fdWrite);
    fdWrite = -1;
  }
}

//-----------------------------------------------------------------------------
void Pipe::mergeRead(const int fd) {
  if (fdRead < 0) {
    throw Error("Pipe.mergeRead() not open");
  }
  if (fd < 0) {
    throw Error("Pipe.mergeRead() invalid destination descriptor");
  }
  if (::dup2(fdRead, fd) != fd) {
    throw Error(Msg() << "Pipe.mergeRead() failed: " << toError(errno));
  }
}

//-----------------------------------------------------------------------------
void Pipe::mergeWrite(const int fd) {
  if (fdWrite < 0) {
    throw Error("Pipe.mergeWrite() not open");
  }
  if (fd < 0) {
    throw Error("Pipe.mergeWrite() invalid destination descriptor");
  }
  if (::dup2(fdWrite, fd) != fd) {
    throw Error(Msg() << "Pipe.mergeWrite() failed: " << toError(errno));
  }
}

//-----------------------------------------------------------------------------
void Pipe::writeln(const std::string& str) const {
  if (fdWrite < 0) {
    throw Error("Pipe.writeln() not open for writing");
  }

  std::string line = str;
  if (!endsWith(line, '\n')) {
    line += '\n';
  }

  if (::write(fdWrite, line.c_str(), line.size()) != int(line.size())) {
    throw Error(Msg() << "Pipe.writeln() failed: " << toError(errno));
  }
}

} // namespace subsim
