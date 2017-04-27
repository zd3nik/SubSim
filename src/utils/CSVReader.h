//----------------------------------------------------------------------------
// CSVReader.h
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//----------------------------------------------------------------------------
#ifndef SUBSIM_CVS_READER_H
#define SUBSIM_CVS_READER_H

#include "Platform.h"
#include "StringUtils.h"

namespace subsim
{

//-----------------------------------------------------------------------------
class CSVReader {
//-----------------------------------------------------------------------------
private: // variables
  bool trim = false;
  char delim = ',';
  unsigned rowPos = 0;
  std::string row;

//-----------------------------------------------------------------------------
public: // constructors
  CSVReader() = default;
  CSVReader(CSVReader&&) noexcept = default;
  CSVReader(const CSVReader& other) = default;
  CSVReader& operator=(CSVReader&&) noexcept = default;
  CSVReader& operator=(const CSVReader& other) = default;

  explicit CSVReader(const std::string& rowStr,
                     const char delim = ',',
                     const bool trim = false)
    : trim(trim),
      delim(delim),
      row(rowStr)
  { }

//-----------------------------------------------------------------------------
public: // getters
  char getDelimiter() const noexcept { return delim; }
  bool isTrim() const noexcept { return trim; }
  std::string remaining() const {
    return (rowPos < row.size()) ? row.substr(rowPos) : "";
  }

//-----------------------------------------------------------------------------
public: // setters
  CSVReader& setDelimiter(const char ch) noexcept {
    delim = ch;
    return (*this);
  }

  CSVReader& setTrim(const bool flag) noexcept {
    trim = flag;
    return (*this);
  }

//-----------------------------------------------------------------------------
public: // methods
  CSVReader& reset() {
    rowPos = 0;
    row.clear();
    return (*this);
  }

  CSVReader& reset(const std::string& rowStr) {
    rowPos = 0;
    row = rowStr;
    return (*this);
  }

  std::vector<std::string> readCells();
  bool readCell(std::string& cell, const std::string& def = "");
  bool readCell(int& cell, const int def = 0);
  bool readCell(unsigned& cell, const unsigned def = 0);
  bool readCell(int64_t& cell, const int64_t def = 0);
  bool readCell(u_int64_t& cell, const u_int64_t def = 0);
  bool readCell(double& cell, const double def = 0);
  bool readCell(bool& cell, const bool def = false);

//-----------------------------------------------------------------------------
public: // operator overloads
  template<typename T>
  CSVReader& operator>>(T& value) {
    readCell(value);
    return (*this);
  }

  CSVReader& operator=(const std::string& row) {
    return reset(row);
  }
};

} // namespace subsim

#endif // SUBSIM_CVS_READER_H

