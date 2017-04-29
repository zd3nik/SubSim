//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#include "Timer.h"
#include <thread>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/time.h>

namespace subsim
{

//-----------------------------------------------------------------------------
const Milliseconds Timer::ZERO_MS = Milliseconds(0);

//-----------------------------------------------------------------------------
const Timestamp Timer::BAD_TIME = Timestamp(-1);

//-----------------------------------------------------------------------------
std::string
Timer::toString(const Milliseconds ms) {
  const Milliseconds h = (ms / ONE_HOUR);
  const Milliseconds m = ((ms % ONE_HOUR) / ONE_MINUTE);
  const Milliseconds s = ((ms % ONE_MINUTE) / ONE_SECOND);
  const Milliseconds p = (ms % ONE_SECOND);
  std::stringstream ss;
  ss << std::setfill('0') << h << ':'
     << std::setw(2) << m << std::setw(0) << ':'
     << std::setw(2) << s << std::setw(0) << '.'
     << std::setw(3) << p;
  return ss.str();
}

//-----------------------------------------------------------------------------
std::string
Timer::toString() const {
  return toString(elapsed());
}

//-----------------------------------------------------------------------------
void
Timer::sleep(const Milliseconds ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

//-----------------------------------------------------------------------------
Timestamp
Timer::now() noexcept {
  timeval tv;
  if (gettimeofday(&tv, nullptr) < 0) {
    return BAD_TIME;
  }
  return static_cast<Timestamp>((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

} // namespace subsim

