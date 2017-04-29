//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_TIMER_H
#define SUBSIM_TIMER_H

#include "Platform.h"
#include "Printable.h"

namespace subsim
{

//-----------------------------------------------------------------------------
typedef int64_t Milliseconds;
typedef Milliseconds Timestamp;

//-----------------------------------------------------------------------------
class Timer : public Printable {
//-----------------------------------------------------------------------------
public: // enums
  enum Interval : Milliseconds {
    ONE_SECOND = 1000,
    ONE_MINUTE = (60 * 1000),
    ONE_HOUR   = (60 * 60 * 1000),
    ONE_DAY    = (24 * 60 * 60 * 1000)
  };

//-----------------------------------------------------------------------------
private: // variables
  Timestamp startTime;
  Timestamp lastTick;

//-----------------------------------------------------------------------------
public: // constructors
  Timer(Timer&&) noexcept = default;
  Timer(const Timer&) noexcept = default;
  Timer& operator=(Timer&&) noexcept = default;
  Timer& operator=(const Timer&) noexcept = default;

  Timer(const Timestamp startTime = now()) noexcept
    : startTime(startTime),
      lastTick(startTime)
  { }

//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override;

//-----------------------------------------------------------------------------
public: // static constants
  static const Milliseconds ZERO_MS;
  static const Timestamp BAD_TIME;

//-----------------------------------------------------------------------------
public: // static methods
  static void sleep(const Milliseconds ms);
  static Timestamp now() noexcept;
  static std::string toString(const Milliseconds ms);

//-----------------------------------------------------------------------------
public: // methods
  void start(const Timestamp timestamp) noexcept {
    lastTick = startTime = timestamp;
  }

  void start() noexcept { start(now()); }
  void tick() noexcept { lastTick = now(); }
  Milliseconds elapsed() const noexcept { return (now() - startTime); }
  Milliseconds tock() const noexcept { return (now() - lastTick); }
  Timestamp lastStartTime() const noexcept { return startTime; }
  Timestamp lastTickTime() const noexcept { return lastTick; }
};

} // namespace subsim

#endif // SUBSIM_TIMER_H

