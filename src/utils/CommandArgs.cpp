//-----------------------------------------------------------------------------
// CommandArgs.cpp
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#include "CommandArgs.h"
#include "Error.h"
#include <mutex>

namespace subsim
{

//-----------------------------------------------------------------------------
static const std::string SWITCH_TOGGLE("--");
static std::mutex ARGS_MUTEX;

//-----------------------------------------------------------------------------
static std::unique_ptr<CommandArgs> instance;

//-----------------------------------------------------------------------------
void
CommandArgs::initialize(const int argc, const char* argv[]) {
  std::lock_guard<std::mutex> lock(ARGS_MUTEX);
  UNUSED(lock);
  instance.reset(new CommandArgs(argc, argv));
}

//-----------------------------------------------------------------------------
const CommandArgs&
CommandArgs::getInstance() {
  std::lock_guard<std::mutex> lock(ARGS_MUTEX);
  UNUSED(lock);
  if (!instance) {
    instance.reset(new CommandArgs(0, nullptr));
  }
  return (*instance);
}

//-----------------------------------------------------------------------------
CommandArgs::CommandArgs(const int argc, const char** argv)
  : initialized((argc > 0) && argv)
{
  if (initialized) {
    progPath = progName = argv[0];
    auto end = progName.find_last_of("/\\");
    if ((end != std::string::npos) && ((end + 1) < progName.size())) {
      progName = progName.substr(end + 1);
    }
    for (int i = 1; i < argc; ++i) {
      args.push_back(argv[i] ? argv[i] : "");
      if ((endSwitch < 0) && (SWITCH_TOGGLE == argv[i])) {
        endSwitch = i;
      }
    }
  }
}

//-----------------------------------------------------------------------------
int
CommandArgs::idxOf(const std::string& arg,
                   const int afterIdx) const noexcept
{
  if (arg.size()) {
    for (size_t idx = (afterIdx < 0) ? 0 : (afterIdx + 1);
         idx < args.size(); ++idx)
    {
      if (arg == args[idx]) {
        return static_cast<int>(idx);
      }
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
int
CommandArgs::idxOf(const std::initializer_list<std::string>& list,
                   const int afterIdx) const noexcept
{
  for (const std::string& arg : list) {
    const int idx = idxOf(arg, afterIdx);
    if (idx >= 0) {
      return idx;
    }
  }
  return -1;
}

//-----------------------------------------------------------------------------
bool
CommandArgs::match(const int idx, const std::string& arg) const {
  return (arg.size() && (arg == get(idx)));
}

//-----------------------------------------------------------------------------
bool
CommandArgs::match(const int idx,
                   const std::initializer_list<std::string>& list) const
{
  for (auto& arg : list) {
    if (match(idx, arg)) {
      return true;
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
std::string
CommandArgs::getStrAfter(const std::string& arg,
                         const std::string& defaultValue,
                         const int afterIdx,
                         int* argIdx,
                         bool* valExists) const
{
  const int idx = idxOf(arg, afterIdx);
  if (argIdx) {
    (*argIdx) = idx;
  }
  if ((idx >= 0) && !isSwitch(idx + 1)) {
    if (valExists) {
      (*valExists) = true;
    }
    return get(idx + 1);
  }
  if (valExists) {
    (*valExists) = false;
  }
  return defaultValue;
}

//-----------------------------------------------------------------------------
std::string
CommandArgs::getStrAfter(const std::initializer_list<std::string>& list,
                         const std::string& def,
                         const int afterIdx,
                         std::string* argPtr,
                         int* argIdx,
                         bool* valExists) const
{
  int firstIdx = -1;
  for (const std::string& arg : list) {
    int idx = -1;
    bool exists = false;
    const std::string val = getStrAfter(arg, def, afterIdx, &idx, &exists);
    if (exists) {
      if (argPtr) {
        (*argPtr) = arg;
      }
      if (argIdx) {
        (*argIdx) = idx;
      }
      if (valExists) {
        (*valExists) = true;
      }
      return val;
    }
    if ((idx >= 0) && (firstIdx < 0)) {
      firstIdx = idx;
    }
  }
  if (argPtr) {
    (*argPtr) = (firstIdx < 0) ? "" : get(firstIdx);
  }
  if (argIdx) {
    (*argIdx) = firstIdx;
  }
  if (valExists) {
    (*valExists) = false;
  }
  return def;
}

//-----------------------------------------------------------------------------
int
CommandArgs::getIntAfter(const std::string& arg,
                         const int defaultValue,
                         const int afterIdx,
                         int* argIdx,
                         bool* valIsInt) const
{
  const int idx = idxOf(arg, afterIdx);
  if (argIdx) {
    (*argIdx) = idx;
  }
  if ((idx >= 0) && !isSwitch(idx + 1)) {
    const std::string val = get(idx + 1);
    if (valIsInt) {
      (*valIsInt) = isInt(val);
    }
    return toInt32(val, defaultValue);
  }
  if (valIsInt) {
    (*valIsInt) = false;
  }
  return defaultValue;
}

//-----------------------------------------------------------------------------
int
CommandArgs::getIntAfter(const std::initializer_list<std::string>& list,
                         const int def,
                         const int afterIdx,
                         std::string* argPtr,
                         int* argIdx,
                         bool* valIsInt) const
{
  int firstIdx = -1;
  for (const std::string& arg : list) {
    int idx = -1;
    bool exists = false;
    const int val = getIntAfter(arg, def, afterIdx, &idx, &exists);
    if (exists) {
      if (argPtr) {
        (*argPtr) = arg;
      }
      if (argIdx) {
        (*argIdx) = idx;
      }
      if (valIsInt) {
        (*valIsInt) = exists;
      }
      return val;
    }
    if ((idx >= 0) && (firstIdx < 0)) {
      firstIdx = idx;
    }
  }
  if (argPtr) {
    (*argPtr) = (firstIdx < 0) ? "" : get(firstIdx);
  }
  if (argIdx) {
    (*argIdx) = firstIdx;
  }
  if (valIsInt) {
    (*valIsInt) = false;
  }
  return def;
}

//-----------------------------------------------------------------------------
unsigned
CommandArgs::getUIntAfter(const std::string& arg,
                          const unsigned defaultValue,
                          const int afterIdx,
                          int* argIdx,
                          bool* valIsUInt) const
{
  const int idx = idxOf(arg, afterIdx);
  if (argIdx) {
    (*argIdx) = idx;
  }
  if ((idx >= 0) && !isSwitch(idx + 1)) {
    const std::string val = get(idx + 1);
    if (valIsUInt) {
      (*valIsUInt) = isUInt(val);
    }
    return toUInt32(val, defaultValue);
  }
  if (valIsUInt) {
    (*valIsUInt) = false;
  }
  return defaultValue;
}

//-----------------------------------------------------------------------------
unsigned
CommandArgs::getUIntAfter(const std::initializer_list<std::string>& list,
                          const unsigned def,
                          const int afterIdx,
                          std::string* argPtr,
                          int* argIdx,
                          bool* valIsUInt) const
{
  int firstIdx = -1;
  for (const std::string& arg : list) {
    int idx = -1;
    bool exists = false;
    const unsigned val = getUIntAfter(arg, def, afterIdx, &idx, &exists);
    if (exists) {
      if (argPtr) {
        (*argPtr) = arg;
      }
      if (argIdx) {
        (*argIdx) = idx;
      }
      if (valIsUInt) {
        (*valIsUInt) = exists;
      }
      return val;
    }
    if ((idx >= 0) && (firstIdx < 0)) {
      firstIdx = idx;
    }
  }
  if (argPtr) {
    (*argPtr) = (firstIdx < 0) ? "" : get(firstIdx);
  }
  if (argIdx) {
    (*argIdx) = firstIdx;
  }
  if (valIsUInt) {
    (*valIsUInt) = false;
  }
  return def;
}

//-----------------------------------------------------------------------------
double
CommandArgs::getDoubleAfter(const std::string& arg,
                            const double defaultValue,
                            const int afterIdx,
                            int* argIdx,
                            bool* valIsDouble) const
{
  const int idx = idxOf(arg, afterIdx);
  if (argIdx) {
    (*argIdx) = idx;
  }
  if ((idx >= 0) && !isSwitch(idx + 1)) {
    const std::string val = get(idx + 1);
    if (valIsDouble) {
      (*valIsDouble) = isFloat(val);
    }
    return toDouble(val, defaultValue);
  }
  if (valIsDouble) {
    (*valIsDouble) = false;
  }
  return defaultValue;
}

//-----------------------------------------------------------------------------
double
CommandArgs::getDoubleAfter(const std::initializer_list<std::string>& list,
                            const double def,
                            const int afterIdx,
                            std::string* argPtr,
                            int* argIdx,
                            bool* valIsDouble) const
{
  int firstIdx = -1;
  for (const std::string& arg : list) {
    int idx = -1;
    bool exists = false;
    const double val = getDoubleAfter(arg, def, afterIdx, &idx, &exists);
    if (exists) {
      if (argPtr) {
        (*argPtr) = arg;
      }
      if (argIdx) {
        (*argIdx) = idx;
      }
      if (valIsDouble) {
        (*valIsDouble) = exists;
      }
      return val;
    }
    if ((idx >= 0) && (firstIdx < 0)) {
      firstIdx = idx;
    }
  }
  if (argPtr) {
    (*argPtr) = (firstIdx < 0) ? "" : get(firstIdx);
  }
  if (argIdx) {
    (*argIdx) = firstIdx;
  }
  if (valIsDouble) {
    (*valIsDouble) = false;
  }
  return def;
}

//-----------------------------------------------------------------------------
bool
CommandArgs::isSwitch(const int idx) const {
  if ((idx >= 0) && (static_cast<unsigned>(idx) < args.size()) &&
      ((endSwitch < 0) || (idx < endSwitch)))
  {
    const std::string str = args[idx];
    return ((str.size() > 1) &&
            ((str[0] == '-') || (str[0] == '+')) &&
        !isdigit(str[1]));
  }
  return false;
}

} // namespace subsim
