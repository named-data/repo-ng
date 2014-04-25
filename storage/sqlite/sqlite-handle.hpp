/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California.
 *
 * This file is part of NDN repo-ng (Next generation of NDN repository).
 * See AUTHORS.md for complete list of repo-ng authors and contributors.
 *
 * repo-ng is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * repo-ng is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * repo-ng, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
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

  virtual size_t
  size();

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
