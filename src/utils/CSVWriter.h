//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef SUBSIM_CSV_WRITER_H
#define SUBSIM_CSV_WRITER_H

#include "Platform.h"
#include "Printable.h"
#include "StringUtils.h"

#include <sstream>

namespace subsim
{

//-----------------------------------------------------------------------------
class CSVWriter : public Printable {
//-----------------------------------------------------------------------------
private: // variables
  bool trim = false;
  char delim = ',';
  unsigned cellCount = 0;
  std::stringstream row;

//-----------------------------------------------------------------------------
public: // constructors
  CSVWriter() = default;
  CSVWriter(CSVWriter&&) noexcept = default;
  CSVWriter& operator=(CSVWriter&&) noexcept = default;
  CSVWriter(const CSVWriter&);
  CSVWriter& operator=(const CSVWriter&);

  explicit CSVWriter(const char delim, const bool trim = false)
    : trim(trim),
      delim(delim)
  { }

  explicit CSVWriter(const std::string& str,
                     const char delim,
                     const bool trim = false)
    : CSVWriter(delim, trim)
  {
    if (str.size()) {
      cellCount++;
      row << str;
    }
  }

 //-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override { return row.str(); }

//-----------------------------------------------------------------------------
public: // getters
  bool isTrim() const noexcept { return trim; }
  char getDelimiter() const noexcept { return delim; }
  unsigned getCellCount() const noexcept { return cellCount; }

//-----------------------------------------------------------------------------
public: // setters
  CSVWriter& setTrim(const bool flag) noexcept {
    trim = flag;
    return (*this);
  }

  CSVWriter& setDelimiter(const char ch) noexcept {
    delim = ch;
    return (*this);
  }

//-----------------------------------------------------------------------------
public: // methods
  CSVWriter& clear() {
    row.str(std::string());
    cellCount = 0;
    return (*this);
  }

  CSVWriter& writeCell(const std::string& strCell);

  CSVWriter& writeCells(const std::vector<std::string>& strCells) {
    for (auto& strCell : strCells) {
      writeCell(strCell);
    }
    return (*this);
  }

//-----------------------------------------------------------------------------
public: // operator overloads
  template<typename T>
  CSVWriter& operator<<(const T& cellData) {
    return writeCell(toStr(cellData));
  }
};

} // namespace subsim

#endif // SUBSIM_CSV_WRITER_H

