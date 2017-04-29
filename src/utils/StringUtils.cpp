//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#include "StringUtils.h"
#include <cstring>

namespace subsim
{

//-----------------------------------------------------------------------------
bool
contains(const std::string& str, const char ch) noexcept {
  return (str.find(ch) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
contains(const std::string& str, const std::string& substr) noexcept {
  return (str.find(substr) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
containsAny(const std::string& str, const std::string& substr) noexcept {
  return (str.find_first_of(substr) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
iContains(const std::string& str, const char ch) {
  return (toUpper(str).find(toupper(ch)) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
iContains(const std::string& str, const std::string& substr) {
  return (toUpper(str).find(toUpper(substr)) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
iContainsAny(const std::string& str, const std::string& substr) {
  return (toUpper(str).find_first_of(toUpper(substr)) != std::string::npos);
}

//-----------------------------------------------------------------------------
bool
iEqual(const std::string& a, const std::string& b) noexcept {
  const unsigned len = a.size();
  if (b.size() != len) {
    return false;
  }
  for (auto ai = a.begin(), bi = b.begin(); ai != a.end(); ++ai, ++bi) {
    if (toupper(*ai) != toupper(*bi)) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
bool
iEqual(const std::string& a, const std::string& b, unsigned len) noexcept {
  if ((a.size() < len) || (b.size() < len)) {
    return false;
  }
  for (auto ai = a.begin(), bi = b.begin(); len-- > 0; ++ai, ++bi) {
    if (toupper(*ai) != toupper(*bi)) {
      return false;
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
int
iCompare(const std::string& a, const std::string& b) {
  for (auto ai = a.begin(), bi = b.begin();
       (ai != a.end()) && (bi != b.end()); ++ai, ++bi)
  {
    const char ac = toupper(*ai);
    const char bc = toupper(*bi);
    if (ac < bc) {
      return -1;
    } else if (ac > bc) {
      return 1;
    }
  }
  return (a.size() < b.size()) ? -1 : (a.size() > b.size());
}

//-----------------------------------------------------------------------------
bool
isEmpty(const std::string& str, const bool trimWhitespace) noexcept {
  if (str.empty()) {
    return true;
  }
  if (trimWhitespace) {
    for (const char ch : str) {
      if (!isspace(ch)) {
        return false;
      }
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
isEmpty(const std::string* str, const bool trimWhitespace) noexcept {
  return str ? isEmpty((*str), trimWhitespace) : true;
}

//-----------------------------------------------------------------------------
bool
iStartsWith(const std::string& str,
            const char ch,
            const bool trimWhitespace) noexcept
{
  if (str.size()) {
    for (char strChar : str) {
      if (toupper(strChar) == (ch)) {
        return true;
      } else if (!trimWhitespace || !isspace(strChar)) {
        break;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
iStartsWith(const std::string& str,
            const std::string& substr,
            const bool trimWhitespace)
{
  if (substr.size() && (str.size() >= substr.size())) {
    unsigned i = 0;
    if (trimWhitespace) {
      while ((i < str.size()) && isspace(str[i])) {
        i++;
      }
    }
    return (((i + substr.size()) <= str.size()) &&
            iEqual(str.substr(i, substr.size()), substr));
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
startsWith(const std::string& str,
           const char ch,
           const bool trimWhitespace) noexcept
{
  if (str.size()) {
    for (char strChar : str) {
      if (strChar == ch) {
        return true;
      } else if (!trimWhitespace || !isspace(strChar)) {
        break;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
startsWith(const std::string& str,
           const std::string& substr,
           const bool trimWhitespace)
{
  if (substr.size() && (str.size() >= substr.size())) {
    unsigned i = 0;
    if (trimWhitespace) {
      while ((i < str.size()) && isspace(str[i])) {
        i++;
      }
    }
    return (((i + substr.size()) <= str.size()) &&
            (str.substr(i, substr.size()) == substr));
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
iEndsWith(const std::string& str,
          const char ch,
          const bool trimWhitespace)
{
  if (str.size()) {
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
      const char strChar = (*it);
      if (toupper(strChar) == toupper(ch)) {
        return true;
      } else if (!trimWhitespace || !isspace(strChar)) {
        break;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
iEndsWith(const std::string& str,
          const std::string& substr,
          const bool trimWhitespace)
{
  if (substr.size() && (str.size() >= substr.size())) {
    unsigned i = (str.size() - 1);
    if (trimWhitespace) {
      while ((i > 0) && isspace(str[i])) {
        i--;
      }
    }
    return ((++i >= substr.size()) &&
            iEqual(str.substr((i - substr.size()), substr.size()), substr));
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
endsWith(const std::string& str,
         const char ch,
         const bool trimWhitespace) noexcept
{
  if (str.size()) {
    for (auto it = str.rbegin(); it != str.rend(); ++it) {
      const char strChar = (*it);
      if (strChar == ch) {
        return true;
      } else if (!trimWhitespace || !isspace(strChar)) {
        break;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
endsWith(const std::string& str,
         const std::string& substr,
         const bool trimWhitespace)
{
  if (substr.size() && (str.size() >= substr.size())) {
    unsigned i = (str.size() - 1);
    if (trimWhitespace) {
      while ((i > 0) && isspace(str[i])) {
        i--;
      }
    }
    return ((++i >= substr.size()) &&
            (str.substr((i - substr.size()), substr.size()) == substr));
  }
  return false;
}

//-----------------------------------------------------------------------------
bool
startsWithNumber(const std::string& str,
                 const bool trimWhitespace) noexcept
{
  if (str.empty()) {
    return false;
  }

  unsigned i = 0;
  if (trimWhitespace) {
    while ((i < str.size()) && isspace(str[i])) {
      i++;
    }
  }

  if (i >= str.size()) {
    return false;
  } else if (isdigit(str[i])) {
    return true;
  } else if (str.size() < (2 + i)) {
    return false;
  } else if ((str[i] != '+') && (str[i] != '-')) {
    return false;
  } else {
    return isdigit(str[i + 1]);
  }
}

//-----------------------------------------------------------------------------
bool
isFloat(const std::string& str) noexcept {
  unsigned signs = 0;
  unsigned digits = 0;
  unsigned dots = 0;
  for (const char ch : str) {
    if (isdigit(ch)) {
      digits++;
    } else if (ch == '.') {
      if (dots++ || !digits) {
        return false;
      }
    } else if ((ch == '+') || (ch == '-')) {
      if (signs++ || digits || dots) {
        return false;
      }
    } else {
      break;
    }
  }
  return digits;
}

//-----------------------------------------------------------------------------
bool
isInt(const std::string& str) noexcept {
  unsigned signs = 0;
  unsigned digits = 0;
  for (const char ch : str) {
    if (isdigit(ch)) {
      digits++;
    } else if (ch == '.') {
      return false;
    } else if ((ch == '+') || (ch == '-')) {
      if (signs++ || digits) {
        return false;
      }
    } else {
      break;
    }
  }
  return digits;
}

//-----------------------------------------------------------------------------
bool
isUInt(const std::string& str) noexcept {
  unsigned signs = 0;
  unsigned digits = 0;
  for (const char ch : str) {
    if (isdigit(ch)) {
      digits++;
    } else if (ch == '.') {
      return false;
    } else if (ch == '+') {
      if (signs++ || digits) {
        return false;
      }
    } else {
      break;
    }
  }
  return digits;
}

//-----------------------------------------------------------------------------
bool
isBool(const std::string& str) noexcept {
  return (iEqual(str, "true") || iEqual(str, "false") ||
          iEqual(str, "yes") || iEqual(str, "no") ||
          iEqual(str, "y") || iEqual(str, "n") ||
          (str == "1") || (str == "0"));
}

//-----------------------------------------------------------------------------
template<typename T>
T convertToInt(const std::string& str, const T def) noexcept {
  if (str.empty()) {
    return def;
  }
  T sign = 0;
  T result = 0;
  bool started = false;
  for (const char ch : str) {
    if (isspace(ch)) {
      if (started) {
        break;
      }
    } else {
      started = true;
      if (ch == '-') {
        if (sign) {
          return def;
        } else {
          sign = -1;
        }
      } else if (ch == '+') {
        if (sign) {
          return def;
        } else {
          sign = 1;
        }
      } else {
        if (!sign) {
          sign = 1;
        }
        if (isdigit(ch)) {
          T tmp = ((10 * result) + (ch - '0'));
          if (tmp >= result) {
            result = tmp;
          } else {
            return def;
          }
        } else {
          return def;
        }
      }
    }
  }
  return (sign * result);
}

//-----------------------------------------------------------------------------
int16_t toInt16(const std::string& str, const int16_t def) noexcept {
  return convertToInt(str, def);
}

//-----------------------------------------------------------------------------
int32_t toInt32(const std::string& str, const int32_t def) noexcept {
  return convertToInt(str, def);
}

//-----------------------------------------------------------------------------
int64_t toInt64(const std::string& str, const int64_t def) noexcept {
  return convertToInt(str, def);
}

//-----------------------------------------------------------------------------
template<typename T>
T convertToUInt(const std::string& str, const T def) noexcept {
  if (str.empty()) {
    return def;
  }
  T sign = 0;
  T result = 0;
  bool started = false;
  for (const char ch : str) {
    if (isspace(ch)) {
      if (started) {
        break;
      }
    } else {
      started = true;
      if (ch == '-') {
        return def;
      } else if (ch == '+') {
        if (sign) {
          return def;
        } else {
          sign = 1;
        }
      } else {
        if (!sign) {
          sign = 1;
        }
        if (isdigit(ch)) {
          T tmp = ((10 * result) + (ch - '0'));
          if (tmp >= result) {
            result = tmp;
          } else {
            return def;
          }
        } else {
          return def;
        }
      }
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
u_int16_t toUInt16(const std::string& str, const u_int16_t def) noexcept {
  return convertToUInt(str, def);
}

//-----------------------------------------------------------------------------
u_int32_t toUInt32(const std::string& str, const u_int32_t def) noexcept {
  return convertToUInt(str, def);
}

//-----------------------------------------------------------------------------
u_int64_t toUInt64(const std::string& str, const u_int64_t def) noexcept {
  return convertToUInt(str, def);
}

//-----------------------------------------------------------------------------
double toDouble(const std::string& str, const double def) noexcept {
  return isFloat(str) ? atof(str.c_str()) : def;
}

//-----------------------------------------------------------------------------
float toFloat(const std::string& str, const float def) noexcept {
  return isFloat(str) ? strtof(str.c_str(), nullptr) : def;
}

//-----------------------------------------------------------------------------
bool toBool(const std::string& str, const bool def) noexcept {
  return (iEqual(str, "true") ||
          iEqual(str, "yes") ||
          iEqual(str, "y") ||
          (isInt(str) ? (toInt64(str) != 0)
                      : isFloat(str) ? (toDouble(str) != 0) : def));
}

//-----------------------------------------------------------------------------
std::string toUpper(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), toupper);
  return str;
}

//-----------------------------------------------------------------------------
std::string toLower(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(), tolower);
  return str;
}

//-----------------------------------------------------------------------------
std::string toError(const int errorNumber) {
  return strerror(errorNumber);
}

//-----------------------------------------------------------------------------
std::string trimStr(const char* str) {
  return str ? trimStr(std::string(str)) : "";
}

//-----------------------------------------------------------------------------
std::string trimStr(const std::string& str) {
  std::string result;
  if (str.size()) {
    result.reserve(str.size());
    const char* begin = str.c_str();
    while (*begin && isspace(*begin)) {
      ++begin;
    }
    const char* end = begin;
    for (const char* p = begin; *p; ++p) {
      if (!isspace(*p)) {
        end = p;
      }
    }
    result.assign(begin, (end + ((*end) != 0)));
  }
  return result;
}

//-----------------------------------------------------------------------------
std::string replace(const std::string& str,
                    const std::string& substr,
                    const std::string& replacement)
{
  std::string result;
  if (str.size() && substr.size() && (substr.size() <= str.size())) {
    auto p = str.find(substr);
    if (p != std::string::npos) {
      if (p > 0) {
        result += str.substr(0, p);
      }
      result += replacement;
      if ((p + substr.size()) < str.size()) {
        result += str.substr(p + substr.size());
      }
    }
  }
  return result;
}

} // namespace subsim

