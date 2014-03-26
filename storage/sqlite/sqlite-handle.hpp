/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_STORAGE_SQLITE_SQLITE_HANDLE_HPP
#define REPO_STORAGE_SQLITE_SQLITE_HANDLE_HPP

#include "../storage-handle.hpp"

#include <string>
#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <algorithm>

namespace repo {

using std::queue;

class SqliteHandle : public StorageHandle
{
public:
  class Error : public StorageHandle::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : StorageHandle::Error(what)
    {
    }
  };

  SqliteHandle();

  explicit
  SqliteHandle(const string& dbPath);

  virtual
  ~SqliteHandle();


  // from StorageHandle

  virtual bool
  insertData(const Data& data);

  virtual bool
  deleteData(const Name& name);

  virtual bool
  readData(const Interest& interest, Data& data);

  virtual bool
  hasName(const Name& name);

  virtual bool
  readNameAny(const Name& name, const Selectors& selectors, vector<Name>& names);

private:
  void
  initializeRepo();

  /**
  * @brief find data with the exact name matched
  * @param[out] data Data matching Interest.
  * @return if no data or something is wrong, return false
  */
  bool
  readData(const Name& name, Data& data);

  /**
  * @brief check whether there is one row with this parentName = parentName in database
  * @return if no data or something is wrong, return false.
  */
  bool
  hasParentName(const Name& parentName) const;

  /**
  * @brief This function is for no selector, it will reply the leftmost data
  * @param[out] data Data matching Interest.
  * @return if no data or something is wrong, return false.
  */
  bool
  readDataPlain(const Name& name, Data& data);

  /**
  * @brief  read data with this prefix or name
  * @param name indicates name or prefix of interest
  * @param[out] names is vector to contain the result of this function.
  * @return success return true, error return false
  */
  bool
  readDataName(const Name& name, vector<Name>& names) const;

  /**
  * @brief  read data with this prefix or name and selectors including  MinSuffixComponents,
  * MaxSuffixComponents, PublisherPublicKeyLocator, and Exclude.
  * This method does not consider ChildSelector and MustBeFresh.
  *
  * @param name indicates name or prefix of interest
  * @param[out] names is vector to contain the result of this function.
  * @return success return true, error return false
  */
  bool
  readNameSelector(const Interest& interest, vector<Name>& names) const;

  /**
  * @brief ChildSelector filter
  * @param names list of candidate names for ChildSelector filter
  * @param[out] resultName is the result of selected name
  * @return success return true, error return false
  */
  bool
  filterNameChild(const Name& name, int childSelector,
                  const vector<Name>& names, Name& resultName);

private:
  sqlite3* m_db;
  string m_dbPath;
};

} // namespace repo

#endif
