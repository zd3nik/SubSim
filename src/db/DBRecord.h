//-----------------------------------------------------------------------------
// DBRecord.h
// Copyright (c) 2016-2017 Shawn Chidester, All rights reserved
//-----------------------------------------------------------------------------
#ifndef XBS_DB_RECORD_H
#define XBS_DB_RECORD_H

#include "Platform.h"
#include "Printable.h"

namespace xbs
{

//-----------------------------------------------------------------------------
class DBRecord : public Printable {
//-----------------------------------------------------------------------------
public: // Printable implementation
  std::string toString() const override { return getID(); }

//-----------------------------------------------------------------------------
public: // abstract methods
  virtual std::string getID() const = 0;
  virtual void clear(const std::string& fld) = 0;

  virtual std::vector<std::string> getStrings(const std::string& fld) const = 0;
  virtual std::string getString(const std::string& fld) const = 0;
  virtual void setString(const std::string& fld, const std::string& val) = 0;
  virtual unsigned addString(const std::string& fld, const std::string& val) = 0;
  virtual unsigned addStrings(const std::string& fld,
                              const std::vector<std::string>& values) = 0;

//-----------------------------------------------------------------------------
public: // virtual methods
  virtual std::vector<int> getInts(const std::string& fld) const;
  virtual int getInt(const std::string& fld) const;
  virtual int incInt(const std::string& fld, const int inc = 1);
  virtual void setInt(const std::string& fld, const int val);
  virtual unsigned addInt(const std::string& fld, const int val);
  virtual unsigned addInts(const std::string& fld,
                           const std::vector<int>& values);

  virtual std::vector<unsigned> getUInts(const std::string& fld) const;
  virtual unsigned getUInt(const std::string& fld) const;
  virtual unsigned incUInt(const std::string& fld, const unsigned inc = 1);
  virtual void setUInt(const std::string& fld, const unsigned val);
  virtual unsigned addUInt(const std::string& fld, const unsigned val);
  virtual unsigned addUInts(const std::string& fld,
                            const std::vector<unsigned>& values);

  virtual std::vector<u_int64_t> getUInt64s(const std::string& fld) const;
  virtual u_int64_t getUInt64(const std::string& fld) const;
  virtual u_int64_t incUInt64(const std::string& fld, const u_int64_t inc = 1);
  virtual void setUInt64(const std::string& fld, const u_int64_t val);
  virtual unsigned addUInt64(const std::string& fld, const u_int64_t val);
  virtual unsigned addUInt64s(const std::string& fld,
                              const std::vector<u_int64_t>& values);

  virtual std::vector<bool> getBools(const std::string& fld) const;
  virtual bool getBool(const std::string& fld) const;
  virtual void setBool(const std::string& fld, const bool val);
  virtual unsigned addBool(const std::string& fld, const bool val);
  virtual unsigned addBools(const std::string& fld,
                            const std::vector<bool>& values);
};

} // namespace xbs

#endif // XBS_DB_RECORD_H

