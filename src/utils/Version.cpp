//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All Rights Reserved.
//-----------------------------------------------------------------------------
#include "Version.h"
#include "CSVReader.h"
#include "CSVWriter.h"

namespace subsim
{

//-----------------------------------------------------------------------------
const Version Version::NO_VERSION;
const Version Version::MIN_VERSION(0);
const Version Version::MAX_VERSION(~0U, ~0U, ~0U);

//-----------------------------------------------------------------------------
unsigned addOther(CSVWriter& ver, std::string& other) {
  CSVReader row(other);
  row.setDelimiter('.').setTrim(true);

  other.clear();

  unsigned parts = 0;
  for (std::string cell; row.readCell(cell); ver << cell) {
    if (parts++) {
      other += '.';
    }
    other += cell;
  }
  return parts;
}

//-----------------------------------------------------------------------------
Version::Version(const std::string& value)
  : str(trimStr(value))
{
  if (str.size()) {
    CSVWriter ver;
    ver.setDelimiter('.');

    CSVReader row(str);
    row.setDelimiter('.').setTrim(true);

    std::string cell;
    while (row.readCell(cell)) {
      ver << cell;
      if ((++parts > 3) || !isUInt(cell)) {
        break;
      } else if (parts == 1) {
        numbers = 1;
        majorNum = toUInt32(cell);
      } else if (parts == 2) {
        numbers = 2;
        minorNum = toUInt32(cell);
      } else if (parts == 3) {
        numbers = 3;
        buildNum = toUInt32(cell);
      } else {
        break;
      }
    }

    other = cell;
    while (row.readCell(cell)) {
      ver << cell;
      parts++;
      other += '.';
      other += cell;
    }

    str = ver.toString();
  }
}

//-----------------------------------------------------------------------------
Version::Version(const unsigned majorValue,
                 const std::string& otherValue)
  : parts(1),
    numbers(1),
    majorNum(majorValue),
    other(trimStr(otherValue))
{
  CSVWriter ver;
  ver.setDelimiter('.') << majorNum;
  if (other.size()) {
    parts += addOther(ver, other);
  }
  str = ver.toString();
}

//-----------------------------------------------------------------------------
Version::Version(const unsigned majorValue,
                 const unsigned minorValue,
                 const std::string& otherValue)
  : parts(2),
    numbers(2),
    majorNum(majorValue),
    minorNum(minorValue),
    other(trimStr(otherValue))
{
  CSVWriter ver;
  ver.setDelimiter('.') << majorNum << minorNum;
  if (other.size()) {
    parts += addOther(ver, other);
  }
  str = ver.toString();
}

//-----------------------------------------------------------------------------
Version::Version(const unsigned majorValue,
                 const unsigned minorValue,
                 const unsigned buildValue,
                 const std::string& otherValue)
  : parts(3),
    numbers(3),
    majorNum(majorValue),
    minorNum(minorValue),
    buildNum(buildValue),
    other(trimStr(otherValue))
{
  CSVWriter ver;
  ver.setDelimiter('.') << majorNum << minorNum << buildNum;
  if (other.size()) {
    parts += addOther(ver, other);
  }
  str = ver.toString();
}

//-----------------------------------------------------------------------------
Version&
Version::operator=(const std::string& value) {
  return (*this) = Version(value);
}

//-----------------------------------------------------------------------------
bool
Version::operator<(const Version& v) const noexcept {
  if ((numbers == 0) && (v.numbers > 0)) {
    return true;
  } else if (majorNum < v.majorNum) {
    return true;
  } else if (majorNum > v.majorNum) {
    return false;
  } else if ((numbers == 1) && (v.numbers > 1)) {
    return true;
  } else if (minorNum < v.minorNum) {
    return true;
  } else if (minorNum > v.minorNum) {
    return false;
  } else if ((numbers == 2) && (v.numbers > 2)) {
    return true;
  } else if (buildNum < v.buildNum) {
    return true;
  } else if (buildNum > v.buildNum) {
    return false;
  } else if (((parts - numbers) == 0) && ((v.parts - v.numbers) > 0)) {
    return true;
  } else {
    return (iCompare(other, v.other) < 0);
  }
}

//-----------------------------------------------------------------------------
bool
Version::operator>(const Version& v) const noexcept {
  if ((numbers > 0) && (v.numbers == 0)) {
    return true;
  } else if (majorNum > v.majorNum) {
    return true;
  } else if (majorNum < v.majorNum) {
    return false;
  } else if ((numbers > 1) && (v.numbers == 1)) {
    return true;
  } else if (minorNum > v.minorNum) {
    return true;
  } else if (minorNum < v.minorNum) {
    return false;
  } else if ((numbers > 2) && (v.numbers == 2)) {
    return true;
  } else if (buildNum > v.buildNum) {
    return true;
  } else if (buildNum < v.buildNum) {
    return false;
  } else if (((parts - numbers) > 0) && ((v.parts - v.numbers) == 0)) {
    return true;
  } else {
    return (iCompare(other, v.other) > 0);
  }
}

//-----------------------------------------------------------------------------
bool
Version::operator==(const Version& v) const noexcept {
  return ((parts == v.parts) && (numbers == v.numbers) &&
          (majorNum == v.majorNum) && (minorNum == v.minorNum) &&
          (buildNum == v.buildNum) && iEqual(other, v.other));
}

} // namespace subsim

