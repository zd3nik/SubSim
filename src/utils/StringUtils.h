//-----------------------------------------------------------------------------
// StringUtils.h
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_STRING_UTILS_H
#define SUBSIM_STRING_UTILS_H

#include "Platform.h"
#include <iomanip>
#include <sstream>

namespace subsim
{

//-----------------------------------------------------------------------------
extern bool contains(const std::string& str,
                     const char ch) noexcept;

//-----------------------------------------------------------------------------
extern bool contains(const std::string& str,
                     const std::string& substr) noexcept;

//-----------------------------------------------------------------------------
extern bool containsAny(const std::string& str,
                        const std::string& chars) noexcept;

//-----------------------------------------------------------------------------
extern bool iContains(const std::string& str,
                      const char ch);

//-----------------------------------------------------------------------------
extern bool iContains(const std::string& str,
                      const std::string& substr);

//-----------------------------------------------------------------------------
extern bool iContainsAny(const std::string& str,
                         const std::string& chars);

//-----------------------------------------------------------------------------
extern bool iEqual(const std::string& str1,
                   const std::string& str2) noexcept;

//-----------------------------------------------------------------------------
extern bool iEqual(const std::string& str1,
                   const std::string& str2,
                   unsigned len) noexcept;

//-----------------------------------------------------------------------------
extern int iCompare(const std::string& str1,
                    const std::string& str2);

//-----------------------------------------------------------------------------
extern bool isEmpty(const std::string& str,
                    const bool trimWhitespace = true) noexcept;

//-----------------------------------------------------------------------------
extern bool isEmpty(const std::string* str,
                    const bool trimWhitespace = true) noexcept;

//-----------------------------------------------------------------------------
extern bool iStartsWith(const std::string& str,
                        const char ch,
                        const bool trimWhitespace = false) noexcept;

//-----------------------------------------------------------------------------
extern bool iStartsWith(const std::string& str,
                        const std::string& substr,
                        const bool trimWhitespace = false);

//-----------------------------------------------------------------------------
extern bool startsWith(const std::string& str,
                       const char ch,
                       const bool trimWhitespace = false) noexcept;

//-----------------------------------------------------------------------------
extern bool startsWith(const std::string& str,
                       const std::string& substr,
                       const bool trimWhitespace = false);

//-----------------------------------------------------------------------------
extern bool iEndsWith(const std::string& str,
                      const char ch,
                      const bool trimWhitespace = false);

//-----------------------------------------------------------------------------
extern bool iEndsWith(const std::string& str,
                      const std::string& substr,
                      const bool trimWhitespace = false);

//-----------------------------------------------------------------------------
extern bool endsWith(const std::string& str,
                     const char ch,
                     const bool trimWhitespace = false) noexcept;

//-----------------------------------------------------------------------------
extern bool endsWith(const std::string& str,
                     const std::string& substr,
                     const bool trimWhitespace = false);

//-----------------------------------------------------------------------------
extern bool startsWithNumber(const std::string& str,
                             const bool trimWhitespace = true) noexcept;

//-----------------------------------------------------------------------------
extern bool isFloat(const std::string& str) noexcept;

//-----------------------------------------------------------------------------
extern bool isInt(const std::string& str) noexcept;

//-----------------------------------------------------------------------------
extern bool isUInt(const std::string& str) noexcept;

//-----------------------------------------------------------------------------
extern bool isBool(const std::string& str) noexcept;

//-----------------------------------------------------------------------------
extern int16_t toInt16(const std::string& str,
                       const int16_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern int32_t toInt32(const std::string& str,
                       const int32_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern int64_t toInt64(const std::string& str,
                       const int64_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern u_int16_t toUInt16(const std::string& str,
                          const u_int16_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern u_int32_t toUInt32(const std::string& str,
                          const u_int32_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern u_int64_t toUInt64(const std::string& str,
                          const u_int64_t def = 0) noexcept;

//-----------------------------------------------------------------------------
extern double toDouble(const std::string& str,
                       const double def = 0) noexcept;

//-----------------------------------------------------------------------------
extern float toFloat(const std::string& str,
                     const float def = 0) noexcept;

//-----------------------------------------------------------------------------
extern bool toBool(const std::string& str,
                   const bool def = false) noexcept;

//-----------------------------------------------------------------------------
extern std::string toUpper(std::string str);

//-----------------------------------------------------------------------------
extern std::string toLower(std::string str);

//-----------------------------------------------------------------------------
extern std::string toError(const int errorNumber);

//-----------------------------------------------------------------------------
extern std::string trimStr(const char* str);

//-----------------------------------------------------------------------------
extern std::string trimStr(const std::string& str);

//-----------------------------------------------------------------------------
extern std::string replace(const std::string& str,
                           const std::string& substr,
                           const std::string& replacement);

//-----------------------------------------------------------------------------
inline std::string toStr(const char x) {
  if ((x >= ' ') & (x <= '~')) {
    return std::string(1, x);
  } else {
    std::stringstream ss;
    ss << '\\' << std::right << std::setfill('0') << std::setw(3)
       << static_cast<unsigned>(x);
    return ss.str();
  }
}

//-----------------------------------------------------------------------------
inline std::string toStr(const char* x) {
  return x ? x : "";
}

//-----------------------------------------------------------------------------
inline std::string toStr(const std::string& x) {
  return x;
}

//-----------------------------------------------------------------------------
inline std::string toStr(const std::vector<char>& x) {
  return std::string(x.begin(), x.end());
}

//-----------------------------------------------------------------------------
inline std::string toStr(const bool x) {
  return x ? "true" : "false";
}

//-----------------------------------------------------------------------------
template<typename T>
inline std::string toStr(const T& x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

//-----------------------------------------------------------------------------
template<typename T>
inline std::string toStr(const T& x, int precision) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(precision) << x;
  return ss.str();
}

//-----------------------------------------------------------------------------
template<typename T>
inline std::string rPad(const T& x,
                        const int width,
                        const char padChar = ' ',
                        const int precision = -1)
{
  std::stringstream ss;
  ss << std::right << std::setfill(padChar) << std::setw(width) << std::fixed;
  if (precision >= 0) {
     ss << std::setprecision(precision);
  }
  ss << x;
  return ss.str();
}

//-----------------------------------------------------------------------------
template<typename T>
inline std::string lPad(const T& x,
                        const int width,
                        const char padChar = ' ',
                        const int precision = -1)
{
  std::stringstream ss;
  ss << std::left << std::setfill(padChar) << std::setw(width) << std::fixed;
  if (precision >= 0) {
     ss << std::setprecision(precision);
  }
  ss << x;
  return ss.str();
}

} // namespace subsim

#endif // SUBSIM_STRING_UTILS_H

