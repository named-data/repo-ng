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

#ifndef REPO_STORAGE_STORAGE_HPP
#define REPO_STORAGE_STORAGE_HPP
#include <string>
#include <iostream>
#include <stdlib.h>
#include "../common.hpp"

namespace repo {

/**
  * @brief Storage is a virtual abstract class which will be called by SqliteStorage
  */
class Storage : noncopyable
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

public:
  class ItemMeta
  {
  public:
    int64_t id;
    Name fullName;
    ndn::ConstBufferPtr keyLocatorHash;
  };

public :

  virtual
  ~Storage()
  {
  };

  /**
   *  @brief  put the data into database
   *  @param  data   the data should be inserted into databse
   */
  virtual int64_t
  insert(const Data& data) = 0;

  /**
   *  @brief  remove the entry in the database by using id
   *  @param  id   id number of entry in the database
   */
  virtual bool
  erase(const int64_t id) = 0;

  /**
   *  @brief  get the data from database
   *  @param  id   id number of each entry in the database, used to find the data
   */
  virtual shared_ptr<Data>
  read(const int64_t id) = 0;

  /**
   *  @brief  return the size of database
   */
  virtual int64_t
  size() = 0;

  /**
   *  @brief enumerate each entry in database and call the function
   *         insertItemToIndex to reubuild index from database
   */
  virtual void
  fullEnumerate(const ndn::function<void(const Storage::ItemMeta)>& f) = 0;

};

} // namespace repo

#endif // REPO_STORAGE_Storage_HPP
