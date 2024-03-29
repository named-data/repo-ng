/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023, Regents of the University of California.
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

#ifndef REPO_REPO_HPP
#define REPO_REPO_HPP

#include "storage/repo-storage.hpp"
#include "storage/sqlite-storage.hpp"

#include "handles/delete-handle.hpp"
#include "handles/read-handle.hpp"
#include "handles/tcp-bulk-insert-handle.hpp"
#include "handles/write-handle.hpp"

#include "common.hpp"

#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator-config.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace repo {

struct RepoConfig
{
  static constexpr size_t DISABLED_SUBSET_LENGTH = -1;

  std::string repoConfigPath;
  std::string dbPath;
  std::vector<ndn::Name> dataPrefixes;
  size_t registrationSubset = DISABLED_SUBSET_LENGTH;
  std::vector<ndn::Name> repoPrefixes;
  std::vector<std::pair<std::string, std::string>> tcpBulkInsertEndpoints;
  uint64_t nMaxPackets;
  boost::property_tree::ptree validatorNode;
};

RepoConfig
parseConfig(const std::string& confPath);

class Repo : noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

public:
  Repo(boost::asio::io_context& io, const RepoConfig& config);

  /// @brief Rebuild index from storage file when repo starts.
  void
  initializeStorage();

  void
  enableListening();

  void
  enableValidation();

private:
  RepoConfig m_config;
  Scheduler m_scheduler;
  Face m_face;
  ndn::mgmt::Dispatcher m_dispatcher;
  std::shared_ptr<Storage> m_store;
  RepoStorage m_storageHandle;
  ndn::KeyChain m_keyChain;
  ndn::security::ValidatorConfig m_validator;

  ReadHandle m_readHandle;
  WriteHandle m_writeHandle;
  DeleteHandle m_deleteHandle;
  TcpBulkInsertHandle m_tcpBulkInsertHandle;
};

} // namespace repo

#endif // REPO_REPO_HPP
