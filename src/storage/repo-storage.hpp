/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019, Regents of the University of California.
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

#ifndef REPO_STORAGE_REPO_STORAGE_HPP
#define REPO_STORAGE_REPO_STORAGE_HPP

#include "storage.hpp"
#include "../repo-command-parameter.hpp"

#include <ndn-cxx/util/signal.hpp>

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
  explicit
  RepoStorage(Storage& store);

  /**
   * @brief Notify about existing data
   *
   * Note, this cannot be in constructor, as have to be called after signal is connected
   */
  void
  notifyAboutExistingData();

  /**
   *  @brief  insert data into repo
   */
  bool
  insertData(const Data& data);

  /**
   *  @brief   delete data from repo
   *  @param   name from interest, use it as a prefix to find entry needed to be erased in repo
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
   *  @brief   read data from repo
   *  @param   interest  used to request data
   *  @return  std::shared_ptr<Data>
   */
  std::shared_ptr<Data>
  readData(const Interest& interest) const;

public:
  ndn::util::Signal<RepoStorage, ndn::Name> afterDataInsertion;
  ndn::util::Signal<RepoStorage, ndn::Name> afterDataDeletion;

private:
  Storage& m_storage;
  const int NOTFOUND = -1;
};

} // namespace repo

#endif // REPO_STORAGE_REPO_STORAGE_HPP
