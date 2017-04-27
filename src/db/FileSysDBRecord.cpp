//-----------------------------------------------------------------------------
// FileSysDBRecord.cpp
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "FileSysDBRecord.h"
#include "Input.h"
#include "Logger.h"
#include "Msg.h"
#include "StringUtils.h"
#include "Error.h"

namespace xbs
{

//-----------------------------------------------------------------------------
typedef std::vector<std::string> StringVector;
typedef std::map<std::string, StringVector> FieldMap;

//-----------------------------------------------------------------------------
FileSysDBRecord::FileSysDBRecord(const std::string& recordID,
                                 const std::string& filePath)
  : recordID(recordID),
    filePath(filePath),
    dirty(false)
{
  load();
}

//-----------------------------------------------------------------------------
void FileSysDBRecord::clear() {
  fieldCache.clear();
  dirty = true;
}

//-----------------------------------------------------------------------------
void FileSysDBRecord::load() {
  if (recordID.empty()) {
    throw Error(Msg() << "Empty " << (*this) << " record ID");
  } else if (filePath.empty()) {
    throw Error(Msg() << "Empty " << (*this) << " file path");
  }

  dirty = false;

  std::ifstream file(filePath.c_str());
  if (!file) {
    Logger::debug() << "File '" << filePath << "' does not exist";
    return;
  } else {
    Logger::debug() << "Loading '" << filePath << "' as record ID '"
                    << recordID << "'";
  }

  unsigned line = 0;
  std::string str;

  while (std::getline(file, str)) {
    line++;
    str = trimStr(str);
    if (str.empty()) {
      continue;
    }

    const auto p = str.find_first_of('=');
    std::string fld = trimStr(str.substr(0, p));
    if (fld.empty()) {
      Logger::error() << filePath << '@' << line << ": " << str;
      continue;
    } else if (startsWith(fld, '#')) {
      continue;
    }

    std::string val = (p != std::string::npos) ? trimStr(str.substr(p+1)) : "";
    auto it = fieldCache.find(fld);
    if (it == fieldCache.end()) {
      it = fieldCache.insert(it, std::make_pair(fld, StringVector()));
    }
    it->second.push_back(val);

    Logger::debug() << "Loaded " << filePath << '@' << line << ": '" << fld
                    << "'='" << val << "'";
  }
}

//-----------------------------------------------------------------------------
void FileSysDBRecord::store(const bool force) {
  if ((dirty | force) && recordID.size() && filePath.size()) {
    std::ofstream file(filePath.c_str());
    if (!file) {
      throw Error(Msg() << "Failed to open '" << filePath << "': "
                  << toError(errno));
    }

    for (auto it = fieldCache.begin(); it != fieldCache.end(); ++it) {
      for (const auto& value : it->second) {
        file << it->first << '=' << value << '\n';
      }
    }

    if (!file.flush()) {
      throw Error(Msg() << "flush(" << (*this) << ") failed: "
                  << toError(errno));
    }
  }
  dirty = false;
}

//-----------------------------------------------------------------------------
void FileSysDBRecord::clear(const std::string& fieldName) {
  const std::string fld = validate(fieldName);
  auto it = fieldCache.find(fld);
  if (it != fieldCache.end()) {
    fieldCache.erase(it);
    dirty = true;
  }
}

//-----------------------------------------------------------------------------
StringVector FileSysDBRecord::getStrings(const std::string& fieldName) const {
  const std::string fld = validate(fieldName);
  auto it = fieldCache.find(fld);
  if (it != fieldCache.end()) {
    return it->second;
  }
  return StringVector();
}

//-----------------------------------------------------------------------------
std::string FileSysDBRecord::getString(const std::string& fieldName) const {
  const std::string fld = validate(fieldName);
  auto it = fieldCache.find(fld);
  if ((it != fieldCache.end()) && (it->second.size())) {
    return it->second.front();
  }
  return std::string();
}

//-----------------------------------------------------------------------------
void FileSysDBRecord::setString(const std::string& fieldName,
                                const std::string& val)
{
  const std::string fld = validate(fieldName, val);
  auto it = fieldCache.find(fld);
  if (it != fieldCache.end()) {
    it->second.clear();
  } else {
    it = fieldCache.insert(fieldCache.end(),
                           std::make_pair(fld, StringVector()));
  }
  it->second.push_back(val);
  dirty = true;
}

//-----------------------------------------------------------------------------
unsigned FileSysDBRecord::addString(const std::string& fieldName,
                               const std::string& val)
{
  const std::string fld = validate(fieldName, val);
  auto it = fieldCache.find(fld);
  if (it == fieldCache.end()) {
    it = fieldCache.insert(it, std::make_pair(fld, StringVector()));
  }
  it->second.push_back(val);
  dirty = true;
  return it->second.size();
}

//-----------------------------------------------------------------------------
unsigned FileSysDBRecord::addStrings(const std::string& fieldName,
                                     const StringVector& values)
{
  const std::string fld = validate(fieldName);
  auto it = fieldCache.find(fld);
  if (it == fieldCache.end()) {
    it = fieldCache.insert(it, std::make_pair(fld, values));
  } else {
    for (const auto& value : values) {
      if (contains(value, '\n')) {
        throw Error("Newlines not supported in field values");
      } else {
        it->second.push_back(value);
      }
    }
  }
  dirty = true;
  return it->second.size();
}

//-----------------------------------------------------------------------------
std::string FileSysDBRecord::validate(const std::string& fieldName,
                                      const std::string& val) const
{
  const std::string fld = trimStr(fieldName);
  if (fld.empty()) {
    throw Error("Empty field names not supported");
  } else if (contains(fld, '\n')) {
    throw Error("Newlines not supported in field names");
  } else if (contains(val, '\n')) {
    throw Error("Newlines not supported in field values");
  }
  return fld;
}

} // namespace xbs
