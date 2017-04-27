//-----------------------------------------------------------------------------
// FileSysDatabase.h
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef XBS_FILE_SYS_DATABASE_H
#define XBS_FILE_SYS_DATABASE_H

#include "Platform.h"
#include "Database.h"
#include "FileSysDBRecord.h"
#include <dirent.h>

namespace xbs
{

//-----------------------------------------------------------------------------
class FileSysDatabase : public Database {
//-----------------------------------------------------------------------------
private: // variables
  DIR* dir = nullptr;
  std::string homeDir = DEFAULT_HOME_DIR;
  std::map<std::string, std::shared_ptr<FileSysDBRecord>> recordCache;

//-----------------------------------------------------------------------------
public: // constructors
  FileSysDatabase() = default;
  FileSysDatabase(FileSysDatabase&&) = delete;
  FileSysDatabase(const FileSysDatabase&) = delete;
  FileSysDatabase& operator=(FileSysDatabase&&) = delete;
  FileSysDatabase& operator=(const FileSysDatabase&) = delete;

//-----------------------------------------------------------------------------
public: // destructor
  virtual ~FileSysDatabase() noexcept { close(); }

//-----------------------------------------------------------------------------
private: // static constants
  static const std::string DEFAULT_HOME_DIR;

//-----------------------------------------------------------------------------
public: // Database::Printable implementation
  std::string toString() const override { return homeDir; }

//-----------------------------------------------------------------------------
public: // Database implementation
  void sync() override;
  bool remove(const std::string& recordID) override;
  std::vector<std::string> getRecordIDs() override;
  std::shared_ptr<DBRecord> get(const std::string& recordID,
                                const bool add) override;

//-----------------------------------------------------------------------------
public: // methods
  void close() noexcept;
  FileSysDatabase& open(const std::string& dbHomeDir);
  std::string getHomeDir() const { return homeDir; }

//-----------------------------------------------------------------------------
public: // operator overloads
  explicit operator bool() const noexcept { return homeDir.size(); }

  //-----------------------------------------------------------------------------
private: // methods
  void openDir(const std::string& path);
  void closeDir() noexcept;
  void clearCache();
  void loadCache();
  void loadRecord(const std::string& recordID);
};

} // namespace xbs

#endif // XBS_FILE_SYS_DATABASE_H
