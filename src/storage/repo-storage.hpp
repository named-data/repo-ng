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

#ifndef REPO_STORAGE_REPO_STORE_HPP
#define REPO_STORAGE_REPO_STORE_HPP

#include "../common.hpp"
#include "storage.hpp"
#include "index.hpp"
#include "../repo-command-parameter.hpp"

#include <ndn-cxx/exclude.hpp>

#include <queue>

namespace repo {

/**
 *  @brief  RepoStorage handles the storage part of whole repo,
 *          including index and database
 */
class RepoStorage : noncopyable
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
  RepoStorage(const int64_t& nMaxPackets, Storage& store);

  /**
   *  @brief  rebuild index from database
   */
  void
  initialize();

  /**
   *  @brief  insert data into repo
   */
  bool
  insertData(const Data& data);

  /**
   *  @brief   delete data from repo
   *  @param   name     used to find entry needed to be erased in repo
   *  @return  if deletion in either index or database fail, return -1,
   *           otherwise return the number of erased entries
   */
  ssize_t
  deleteData(const Name& name);

  /**
   *  @brief   delete data from repo
   *  @param   interest used to find entry needed to be erased in repo
   *  @return  if deletion in either index or database fail, return -1,
   *           otherwise return the number of erased entries
   */
  ssize_t
  deleteData(const Interest& interest);

  /**
   *  @brief  read data from repo
   *  @param   interest  used to request data
   *  @return  std::shared_ptr<Data>
   */
  std::shared_ptr<Data>
  readData(const Interest& interest) const;

private:
  Index m_index;
  Storage& m_storage;
};

} // namespace repo

#endif // REPO_REPO_STORE_HPP
