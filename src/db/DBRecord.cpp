//-----------------------------------------------------------------------------
// DBRecord.cpp
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "DBRecord.h"
#include "StringUtils.h"

namespace xbs
{

//-----------------------------------------------------------------------------
std::vector<int> DBRecord::getInts(const std::string& fld) const {
  std::vector<std::string> strValues = getStrings(fld);
  std::vector<int> values;
  values.reserve(strValues.size());
  for (const auto& str : strValues) {
    values.push_back(toInt32(str));
  }
  return values;
}

//-----------------------------------------------------------------------------
int DBRecord::getInt(const std::string& fld) const {
  return toInt32(getString(fld));
}

//-----------------------------------------------------------------------------
int DBRecord::incInt(const std::string& fld, const int inc) {
  int val = getInt(fld);
  setInt(fld, (val + inc));
  return (val + inc);
}

//-----------------------------------------------------------------------------
void DBRecord::setInt(const std::string& fld, const int val) {
  setString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addInt(const std::string& fld, const int val) {
  return addString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addInts(const std::string& fld, const std::vector<int>& values) {
  std::vector<std::string> strValues;
  strValues.reserve(values.size());
  for (const auto& val : values) {
    strValues.push_back(toStr(val));
  }
  return addStrings(fld, strValues);
}

//-----------------------------------------------------------------------------
std::vector<unsigned> DBRecord::getUInts(const std::string& fld) const {
  std::vector<std::string> strValues = getStrings(fld);
  std::vector<unsigned> values;
  values.reserve(strValues.size());
  for (const auto& str : strValues) {
    values.push_back(toUInt32(str));
  }
  return values;
}

//-----------------------------------------------------------------------------
unsigned DBRecord::getUInt(const std::string& fld) const {
  return toUInt32(getString(fld));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::incUInt(const std::string& fld, const unsigned inc) {
  unsigned val = getUInt(fld);
  setUInt(fld, (val + inc));
  return (val + inc);
}

//-----------------------------------------------------------------------------
void DBRecord::setUInt(const std::string& fld, const unsigned val) {
  setString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addUInt(const std::string& fld, const unsigned val) {
  return addString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addUInts(const std::string& fld,
                       const std::vector<unsigned>& values)
{
  std::vector<std::string> strValues;
  strValues.reserve(values.size());
  for (const auto& val : values) {
    strValues.push_back(toStr(val));
  }
  return addStrings(fld, strValues);
}

//-----------------------------------------------------------------------------
std::vector<u_int64_t> DBRecord::getUInt64s(const std::string& fld) const {
  std::vector<std::string> strValues = getStrings(fld);
  std::vector<u_int64_t> values;
  values.reserve(strValues.size());
  for (const auto& str : strValues) {
    values.push_back(toUInt64(str));
  }
  return values;
}

//-----------------------------------------------------------------------------
u_int64_t DBRecord::getUInt64(const std::string& fld) const {
  return toUInt64(getString(fld));
}

//-----------------------------------------------------------------------------
u_int64_t DBRecord::incUInt64(const std::string& fld, const u_int64_t inc) {
  u_int64_t val = getUInt64(fld);
  setUInt64(fld, (val + inc));
  return (val + inc);
}

//-----------------------------------------------------------------------------
void DBRecord::setUInt64(const std::string& fld, const u_int64_t val) {
  setString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addUInt64(const std::string& fld, const u_int64_t val) {
  return addString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addUInt64s(const std::string& fld,
                         const std::vector<u_int64_t>& values)
{
  std::vector<std::string> strValues;
  strValues.reserve(values.size());
  for (const auto& val : values) {
    strValues.push_back(toStr(val));
  }
  return addStrings(fld, strValues);
}

//-----------------------------------------------------------------------------
std::vector<bool> DBRecord::getBools(const std::string& fld) const {
  std::vector<std::string> strValues = getStrings(fld);
  std::vector<bool> values;
  values.reserve(strValues.size());
  for (const auto& str : strValues) {
    values.push_back(toBool(str));
  }
  return values;
}

//-----------------------------------------------------------------------------
bool DBRecord::getBool(const std::string& fld) const {
  return toBool(getString(fld));
}

//-----------------------------------------------------------------------------
void DBRecord::setBool(const std::string& fld, const bool val) {
  setString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addBool(const std::string& fld, const bool val) {
  return addString(fld, toStr(val));
}

//-----------------------------------------------------------------------------
unsigned DBRecord::addBools(const std::string& fld,
                       const std::vector<bool>& values)
{
  std::vector<std::string> strValues;
  strValues.reserve(values.size());
  for (const auto& val : values) {
    strValues.push_back(toStr(val));
  }
  return addStrings(fld, strValues);
}

} // namespace xbs

