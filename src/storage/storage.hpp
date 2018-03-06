/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2018, Regents of the University of California.
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

#include "../common.hpp"
#include <string>
#include <iostream>
#include <stdlib.h>

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
  virtual
  ~Storage() = default;

  /**
   *  @brief  put the data into database
   *  @param  data   the data should be inserted into databse
   */
  virtual int64_t
  insert(const Data& data) = 0;

  /**
   *  @brief  remove the entry in the database by full name
   *  @param  full name   full name of the data
   */
  virtual bool
  erase(const Name& name) = 0;

  /**
   *  @brief  get the data from database
   *  @param  full name   full name of the data
   */
  virtual std::shared_ptr<Data>
  read(const Name& name) = 0;

  /**
   *  @brief  check if database already has the data
   *  @param  full name   full name of the data
   */
  virtual bool
  has(const Name& name) = 0;

  /**
   *  @brief  find the data in database by full name and return it
   *  @param  full name   full name of the data
   */
  virtual std::shared_ptr<Data>
  find(const Name& name, bool exactMatch = false) = 0;

  /**
   *  @brief  return the size of database
   */
  virtual uint64_t
  size() = 0;
};

} // namespace repo

#endif // REPO_STORAGE_STORAGE_HPP
