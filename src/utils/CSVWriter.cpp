//----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//----------------------------------------------------------------------------
#include "CSVWriter.h"

namespace subsim
{

//-----------------------------------------------------------------------------
CSVWriter::CSVWriter(const CSVWriter& other)
    : trim(other.trim),
      delim(other.delim),
      cellCount(other.cellCount)
{
    const std::string s = other.row.str();
    if (s.size()) {
        row << s;
    }
}

//-----------------------------------------------------------------------------
CSVWriter&
CSVWriter::operator=(const CSVWriter& other) {
    trim = other.trim;
    delim = other.delim;
    cellCount = other.cellCount;

    const std::string s = other.row.str();
    if (s.size()) {
        row << s;
    }

    return (*this);
}

//-----------------------------------------------------------------------------
CSVWriter&
CSVWriter::writeCell(const std::string& str) {
    if (cellCount++ && delim) {
        row << delim;
    }
    row << str;
    return (*this);
}

} // namespace subsim
