//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_COMMANDARGS_H
#define SUBSIM_COMMANDARGS_H

#include "Platform.h"
#include "StringUtils.h"
#include <initializer_list>

namespace subsim
{

//-----------------------------------------------------------------------------
class CommandArgs {
//-----------------------------------------------------------------------------
private: // variables
  const bool initialized;
  std::vector<std::string> args;
  std::string progPath;
  std::string progName;
  int endSwitch = -1;

//-----------------------------------------------------------------------------
private: // constructors
  explicit CommandArgs(const int argc, const char** argv);
  CommandArgs(CommandArgs&&) = delete;
  CommandArgs(const CommandArgs&) = delete;
  CommandArgs& operator=(CommandArgs&&) = delete;
  CommandArgs& operator=(const CommandArgs&) = delete;

//-----------------------------------------------------------------------------
public: // static methods
  static void initialize(const int argc, const char* argv[]);
  static const CommandArgs& getInstance();

//-----------------------------------------------------------------------------
public: // methods
  bool isInitialized() const noexcept { return initialized; }
  bool isSwitch(const int idx) const;

  std::string getProgram() const { return progPath; }
  std::string getProgramName() const { return progName; }

  int getCount() const noexcept { return static_cast<int>(args.size()); }
  int idxOf(const std::string& arg, const int afterIdx = -1) const noexcept;
  int idxOf(const std::initializer_list<std::string>& args,
              const int afterIdx = -1) const noexcept;

  bool match(const int idx, const std::string& arg) const;
  bool match(const int idx,
             const std::initializer_list<std::string>& args) const;


  std::string get(const int idx) const {
    return ((idx < 0) || (static_cast<unsigned>(idx) >= args.size()))
        ? ""
        : args[idx];
  }

  bool has(const std::string& arg,
           const int afterIdx = -1,
           int* argIdx = nullptr) const noexcept
  {
    int idx = idxOf(arg, afterIdx);
    if (argIdx) {
      (*argIdx) = idx;
    }
    return (idx >= 0);
  }

  bool has(const std::initializer_list<std::string>& args,
           const int afterIdx = -1,
           int* argIdx = nullptr) const noexcept
  {
    int idx = idxOf(args, afterIdx);
    if (argIdx) {
      (*argIdx) = idx;
    }
    return (idx >= 0);
  }

  std::string getStrAfter(const std::string& arg,
                          const std::string& defaultValue = "",
                          const int afterIdx = -1,
                          int* argIdx= nullptr,
                          bool* valExists = nullptr) const;

  std::string getStrAfter(const std::initializer_list<std::string>& args,
                          const std::string& defaultValue = "",
                          const int afterIdx = -1,
                          std::string* arg = nullptr,
                          int* argIdx = nullptr,
                          bool* valExists = nullptr) const;

  int getIntAfter(const std::string& arg,
                  const int defaultValue = 0,
                  const int afterIdx = -1,
                  int* argIdx= nullptr,
                  bool* valIsInt = nullptr) const;

  int getIntAfter(const std::initializer_list<std::string>& args,
                  const int defaultValue = 0,
                  const int afterIdx = -1,
                  std::string* arg = nullptr,
                  int* argIdx = nullptr,
                  bool* valIsInt = nullptr) const;

  unsigned getUIntAfter(const std::string& arg,
                        const unsigned defaultValue = 0,
                        const int afterIdx = -1,
                        int* argIdx= nullptr,
                        bool* valIsUInt = nullptr) const;

  unsigned getUIntAfter(const std::initializer_list<std::string>& args,
                        const unsigned defaultValue = 0,
                        const int afterIdx = -1,
                        std::string* arg = nullptr,
                        int* argIdx = nullptr,
                        bool* valIsUInt = nullptr) const;

  double getDoubleAfter(const std::string& arg,
                        const double defaultValue = 0,
                        const int afterIdx = -1,
                        int* argIdx= nullptr,
                        bool* valIsDbl = nullptr) const;

  double getDoubleAfter(const std::initializer_list<std::string>& args,
                        const double defaultValue = 0,
                        const int afterIdx = -1,
                        std::string* arg = nullptr,
                        int* argIdx = nullptr,
                        bool* valIsDbl = nullptr) const;

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return isInitialized(); }
};

} // namespace subsim

#endif // SUBSIM_COMMANDARGS_H
