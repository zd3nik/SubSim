//----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//----------------------------------------------------------------------------
#include "CSVReader.h"

namespace subsim
{

//-----------------------------------------------------------------------------
std::vector<std::string>
CSVReader::readCells() {
    std::vector<std::string> cells;
    std::string cell;
    while (readCell(cell)) {
        cells.push_back(cell);
    }
    return std::move(cells);
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(std::string& value, const std::string& def) {
    value.clear();
    if (rowPos >= row.size()) {
        value = def;
        return false;
    }
    if (trim) {
        while ((rowPos < row.size()) && isspace(row[rowPos])) {
            rowPos++;
        }
    }

    while (rowPos < row.size()) {
        char ch = row[rowPos++];
        if (ch == delim) {
            break;
        } else {
            value.append(1, ch);
        }
    }

    if (trim) {
        value = trimStr(value);
    }
    return true;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(int& cell, const int def) {
    std::string str;
    if (readCell(str)) {
        cell = toInt32(str, def);
        return true;
    }
    cell = def;
    return false;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(unsigned& cell, const unsigned def) {
    std::string str;
    if (readCell(str)) {
        cell = toUInt32(str, def);
        return true;
    }
    cell = def;
    return false;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(int64_t& cell, const int64_t def) {
    std::string str;
    if (readCell(str)) {
        cell = toInt64(str, def);
        return true;
    }
    cell = def;
    return false;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(u_int64_t& cell, const u_int64_t def) {
    std::string str;
    if (readCell(str)) {
        cell = toUInt64(str, def);
        return true;
    }
    cell = def;
    return false;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(double& cell, const double def) {
    std::string str;
    if (readCell(str)) {
        cell = toDouble(str, def);
        return true;
    }
    cell = def;
    return false;
}

//-----------------------------------------------------------------------------
bool
CSVReader::readCell(bool& cell, const bool def) {
    std::string str;
    if (readCell(str)) {
        cell = toBool(str, def);
        return true;
    }
    cell = def;
    return false;
}

} // namespace subsim

