//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "utils/Platform.h"
#include "utils/CommandArgs.h"
#include "utils/Logger.h"
#include "utils/Screen.h"
#include "subsim/Server.h"
#include <csignal>

using namespace subsim;

//-----------------------------------------------------------------------------
void termSizeChanged(int) {
  Screen::get(true);
}

//-----------------------------------------------------------------------------
int main(const int argc, const char* argv[]) {
  try {
    initRandom();
    CommandArgs::initialize(argc, argv);
    Server server;

    signal(SIGWINCH, termSizeChanged);
    signal(SIGPIPE, SIG_IGN);

    if (!server.init()) {
      return 1;
    }

    while (server.run() && server.isRepeatOn()) { }
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unhandles exception" << std::endl;
  }
  return 1;
}
