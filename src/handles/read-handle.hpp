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

#ifndef REPO_HANDLES_READ_HANDLE_HPP
#define REPO_HANDLES_READ_HANDLE_HPP

#include "common.hpp"

#include "storage/repo-storage.hpp"
#include "repo-command-response.hpp"
#include "repo-command-parameter.hpp"
#include "repo-command.hpp"

namespace repo {

class ReadHandle : public noncopyable
{

public:
  using DataPrefixRegistrationCallback = std::function<void(const ndn::Name&)>;
  using DataPrefixUnregistrationCallback = std::function<void(const ndn::Name&)>;
  struct RegisteredDataPrefix
  {
    const ndn::RegisteredPrefixId* prefixId;
    int useCount;
  };

  ReadHandle(Face& face, RepoStorage& storageHandle, size_t prefixSubsetLength);

  void
  listen(const Name& prefix);

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  const std::map<ndn::Name, RegisteredDataPrefix>&
  getRegisteredPrefixes()
  {
    return m_insertedDataPrefixes;
  }

  /**
   * @param name Full name of the deleted Data
   */
  void
  onDataDeleted(const Name& name);

  /**
   * @param after Do something after successfully registering the data prefix
   */
  void
  onDataInserted(const Name& name);

  void
  connectAutoListen();

private:
  /**
   * @brief Read data from backend storage
   */
  void
  onInterest(const Name& prefix, const Interest& interest);

  void
  onRegisterFailed(const Name& prefix, const std::string& reason);

private:
  size_t m_prefixSubsetLength;
  std::map<ndn::Name, RegisteredDataPrefix> m_insertedDataPrefixes;
  ndn::util::signal::ScopedConnection afterDataDeletionConnection;
  ndn::util::signal::ScopedConnection afterDataInsertionConnection;
  Face& m_face;
  RepoStorage& m_storageHandle;
};

} // namespace repo

#endif // REPO_HANDLES_READ_HANDLE_HPP
