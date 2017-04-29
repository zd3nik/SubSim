//-----------------------------------------------------------------------------
// Copyright (c) 2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef SUBSIM_DATABASE_H
#define SUBSIM_DATABASE_H

#include "utils/Platform.h"
#include "utils/Printable.h"
#include "DBRecord.h"

namespace subsim
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

} // namespace subsim

#endif // SUBSIM_DATABASE_H
