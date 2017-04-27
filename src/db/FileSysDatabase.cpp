//-----------------------------------------------------------------------------
// FileSysDatabase.cpp
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#include "FileSysDatabase.h"
#include "FileSysDBRecord.h"
#include "Logger.h"
#include "Msg.h"
#include "StringUtils.h"
#include "Error.h"
#include <sys/stat.h>

namespace xbs
{

//-----------------------------------------------------------------------------
const std::string FileSysDatabase::DEFAULT_HOME_DIR = "./db";

//-----------------------------------------------------------------------------
void FileSysDatabase::close() noexcept {
  closeDir();
  try { homeDir.clear(); } catch (...) { ASSERT(false); }
  recordCache.clear();
}

//-----------------------------------------------------------------------------
void FileSysDatabase::closeDir() noexcept {
  if (dir) {
    closedir(dir);
    dir = nullptr;
  }
}

//-----------------------------------------------------------------------------
void FileSysDatabase::openDir(const std::string& path) {
  if (dir) {
    throw Error("FileSysDatabase.openDir() dir is already open");
  } else if (isEmpty(path)) {
    throw Error("FileSysDatabase.openDir() empty path");
  }

  int noents = 0;
  while (!(dir = opendir(path.c_str()))) {
    if ((errno != ENOENT) || (++noents > 1)) {
      throw Error(Msg() << "opendir(" << path << ") failed: "
                  << toError(errno));
    } else if (mkdir(path.c_str(), 0750) != 0) {
      throw Error(Msg() << "mkdir(" << path << ") failed: " << toError(errno));
    }
  }

  homeDir = path;
}

//-----------------------------------------------------------------------------
FileSysDatabase& FileSysDatabase::open(const std::string& dbURI) {
  close();
  try {
    openDir(isEmpty(dbURI) ? DEFAULT_HOME_DIR : dbURI);
    loadCache();
    closeDir();
  }
  catch (...) {
    close();
    throw;
  }
  return (*this);
}

//-----------------------------------------------------------------------------
void FileSysDatabase::loadCache() {
  if (!dir) {
    throw Error("FileSysDatabase.loadCache() dir not open");
  }
  for (dirent* entry = readdir(dir); entry; entry = readdir(dir)) {
    std::string name = entry->d_name;
    if ((name.size() > 4) && isalnum(name[0]) && iEndsWith(name, ".ini")) {
      loadRecord(name.substr(0, (name.size() - 4)));
    }
  }
}

//-----------------------------------------------------------------------------
void FileSysDatabase::loadRecord(const std::string& recordID) {
  std::string path = (homeDir + "/" + recordID + ".ini");
  try {
    recordCache[recordID] = std::make_shared<FileSysDBRecord>(recordID, path);
  }
  catch (const std::exception& e) {
    Logger::error() << "Failed to load " << path << ": " << e.what();
  }
  catch (...) {
    Logger::error() << "Failed to load " << path << ": unhandled exception";
  }
}

//-----------------------------------------------------------------------------
void FileSysDatabase::sync() {
  try {
    for (auto it = recordCache.begin(); it != recordCache.end(); ++it) {
      auto rec = it->second;
      if (rec) {
        rec->store();
      } else {
        Logger::error() << "Null DB record for id '" << it->first << "'";
      }
    }
  }
  catch (const std::exception& e) {
    Logger::error() << "Failed to sync '" << homeDir << "' database: "
                    << e.what();
  }
  catch (...) {
    Logger::error() << "Failed to sync '" << homeDir << "' database";
  }
}

//-----------------------------------------------------------------------------
bool FileSysDatabase::remove(const std::string& recordID) {
  bool ok = false;
  auto it = recordCache.find(recordID);
  if (it != recordCache.end()) {
    try {
      auto rec = it->second;
      if (!rec) {
        throw Error(Msg() << "Null " << (*this) << " record");
      } else if (rec->getFilePath().empty()) {
        throw Error(Msg() << "Empty " << (*rec) << " file path");
      } else if (unlink(rec->getFilePath().c_str()) != 0) {
        throw Error(Msg() << "unlink(" << rec->getFilePath() << ") failed: "
                    << toError(errno));
      }
      ok = true;
    }
    catch (const std::exception& e) {
      Logger::error() << "Failed to remove '" << homeDir << "/" << recordID
                      << "': " << e.what();
    }
    catch (...) {
      Logger::error() << "Failed to remove '" << homeDir << "/" << recordID
                      << "'";
    }
    recordCache.erase(it);
  }
  return ok;
}

//-----------------------------------------------------------------------------
std::shared_ptr<DBRecord> FileSysDatabase::get(const std::string& recordID,
                                               const bool add)
{
  auto it = recordCache.find(recordID);
  if (it != recordCache.end()) {
    return it->second;
  } else if (add) {
    std::string path = (homeDir + "/" + recordID + ".ini");
    recordCache[recordID] = std::make_shared<FileSysDBRecord>(recordID, path);
    return recordCache[recordID];
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
std::vector<std::string> FileSysDatabase::getRecordIDs() {
  std::vector<std::string> recordIDs;
  recordIDs.reserve(recordCache.size());
  for (auto it = recordCache.begin(); it != recordCache.end(); ++it) {
    recordIDs.push_back(it->first);
  }
  return recordIDs;
}

} // namespace xbs
