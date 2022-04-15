/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022, Regents of the University of California.
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

#include <sqlite3.h>

namespace repo {

class SqliteStorage : public Storage
{
public:
  explicit
  SqliteStorage(const std::string& dbPath);

  ~SqliteStorage() override;

  /**
   *  @brief  put the data into database
   *  @param  data     the data should be inserted into databse
   *  @return int64_t  the id number of each entry in the database
   */
  int64_t
  insert(const Data& data) override;

  /**
   *  @brief  remove the entry in the database by using name as index
   *  @param  name   name of the data
   */
  bool
  erase(const Name& name) override;

  std::shared_ptr<Data>
  read(const Name& name) override;

  bool
  has(const Name& name) override;

  std::shared_ptr<Data>
  find(const Name& name, bool exactMatch = false) override;

  void
  forEach(const std::function<void(const Name&)>& f) override;

  /**
   *  @brief  return the size of database
   */
  uint64_t
  size() override;

private:
  void
  initializeRepo();

private:
  sqlite3* m_db;
  std::string m_dbPath;
};

} // namespace repo

#endif // REPO_STORAGE_SQLITE_STORAGE_HPP
