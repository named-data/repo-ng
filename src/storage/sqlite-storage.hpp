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

#ifndef REPO_STORAGE_SQLITE_STORAGE_HPP
#define REPO_STORAGE_SQLITE_STORAGE_HPP

#include "storage.hpp"
#include "index.hpp"
#include <string>
#include <iostream>
#include <sqlite3.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <algorithm>

namespace repo {

using std::queue;

class SqliteStorage : public Storage
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  explicit
  SqliteStorage(const string& dbPath);

  virtual
  ~SqliteStorage();

  /**
   *  @brief  put the data into database
   *  @param  data     the data should be inserted into databse
   *  @return int64_t  the id number of each entry in the database
   */
  virtual int64_t
  insert(const Data& data);

  /**
   *  @brief  remove the entry in the database by using id
   *  @param  id   id number of each entry in the database
   */
  virtual bool
  erase(const int64_t id);

  /**
   *  @brief  get the data from database
   *  @para   id   id number of each entry in the database, used to find the data
   */
  virtual shared_ptr<Data>
  read(const int64_t id);

  /**
   *  @brief  return the size of database
   */
  virtual int64_t
  size();

  /**
   *  @brief enumerate each entry in database and call the function
   *         insertItemToIndex to reubuild index from database
   */
  void
  fullEnumerate(const ndn::function<void(const Storage::ItemMeta)>& f);

private:
  void
  initializeRepo();

private:
  sqlite3* m_db;
  string m_dbPath;
  int64_t m_size;
};


} // namespace repo

#endif // REPO_STORAGE_SQLITE_STORAGE_HPP
