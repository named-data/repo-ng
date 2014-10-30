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

#ifndef REPO_REPO_HPP
#define REPO_REPO_HPP

//#include "storage/repo_storage.hpp"
#include "storage/sqlite-storage.hpp"
#include "storage/repo-storage.hpp"

#include "handles/read-handle.hpp"
#include "handles/write-handle.hpp"
#include "handles/watch-handle.hpp"
#include "handles/delete-handle.hpp"
#include "handles/tcp-bulk-insert-handle.hpp"

#include "common.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace repo {

struct RepoConfig
{
  std::string repoConfigPath;
  //StorageMethod storageMethod; This will be implemtented if there is other method.
  std::string dbPath;
  std::vector<ndn::Name> dataPrefixes;
  std::vector<ndn::Name> repoPrefixes;
  std::vector<std::pair<std::string, std::string> > tcpBulkInsertEndpoints;
  int64_t nMaxPackets;
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
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  Repo(boost::asio::io_service& ioService, const RepoConfig& config);

  //@brief rebuild index from storage file when repo starts.
  void
  initializeStorage();

  void
  enableListening();

  void
  enableValidation();

private:
  RepoConfig m_config;
  ndn::Scheduler m_scheduler;
  ndn::Face m_face;
  std::shared_ptr<Storage> m_store;
  RepoStorage m_storageHandle;
  KeyChain m_keyChain;
  ValidatorConfig m_validator;
  ReadHandle m_readHandle;
  WriteHandle m_writeHandle;
  WatchHandle m_watchHandle;
  DeleteHandle m_deleteHandle;
  TcpBulkInsertHandle m_tcpBulkInsertHandle;
};

} // namespace repo

#endif // REPO_REPO_HPP
