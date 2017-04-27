//-----------------------------------------------------------------------------
// Database.h
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef XBS_DATABASE_H
#define XBS_DATABASE_H

#include "Platform.h"
#include "DBRecord.h"
#include "Printable.h"

namespace xbs
{

//-----------------------------------------------------------------------------
class Database : public Printable {
//-----------------------------------------------------------------------------
public: // destructor
  virtual ~Database() { }

//-----------------------------------------------------------------------------
public: // abstract methods
  virtual void sync() = 0;
  virtual bool remove(const std::string& recordID) = 0;
  virtual std::vector<std::string> getRecordIDs() = 0;
  virtual std::shared_ptr<DBRecord> get(const std::string& recordID,
                                        const bool add) = 0;
};

} // namespace xbs

#endif // XBS_DATABASE_H
