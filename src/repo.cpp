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

#include "repo.hpp"
#include "storage/sqlite-storage.hpp"

namespace repo {

RepoConfig
parseConfig(const std::string& configPath)
{
  if (configPath.empty()) {
    std::cerr << "configuration file path is empty" << std::endl;
  }

  std::ifstream fin(configPath.c_str());
  if (!fin.is_open())
    throw Repo::Error("failed to open configuration file '"+ configPath +"'");

  using namespace boost::property_tree;
  ptree propertyTree;
  try {
    read_info(fin, propertyTree);
  }
  catch (ptree_error& e) {
    throw Repo::Error("failed to read configuration file '"+ configPath +"'");
  }

  ptree repoConf = propertyTree.get_child("repo");

  RepoConfig repoConfig;
  repoConfig.repoConfigPath = configPath;

  ptree dataConf = repoConf.get_child("data");
  for (ptree::const_iterator it = dataConf.begin();
       it != dataConf.end();
       ++it)
  {
    if (it->first == "prefix")
      repoConfig.dataPrefixes.push_back(Name(it->second.get_value<std::string>()));
    else
      throw Repo::Error("Unrecognized '" + it->first + "' option in 'data' section in "
                        "configuration file '"+ configPath +"'");
  }

  ptree commandConf = repoConf.get_child("command");
  for (ptree::const_iterator it = commandConf.begin();
       it != commandConf.end();
       ++it)
  {
    if (it->first == "prefix")
      repoConfig.repoPrefixes.push_back(Name(it->second.get_value<std::string>()));
    else
      throw Repo::Error("Unrecognized '" + it->first + "' option in 'command' section in "
                        "configuration file '"+ configPath +"'");
  }

  ptree tcpBulkInsert = repoConf.get_child("tcp_bulk_insert");
  bool isTcpBulkEnabled = false;
  std::string host = "localhost";
  std::string port = "7376";
  for (ptree::const_iterator it = tcpBulkInsert.begin();
       it != tcpBulkInsert.end();
       ++it)
  {
    isTcpBulkEnabled = true;

    // tcp_bulk_insert {
    //   host "localhost"  ; IP address or hostname to listen on
    //   port 7635  ; Port number to listen on
    // }
    if (it->first == "host") {
      host = it->second.get_value<std::string>();
    }
    else if (it->first == "port") {
      port = it->second.get_value<std::string>();
    }
    else
      throw Repo::Error("Unrecognized '" + it->first + "' option in 'tcp_bulk_insert' section in "
                        "configuration file '"+ configPath +"'");
  }
  if (isTcpBulkEnabled) {
    repoConfig.tcpBulkInsertEndpoints.push_back(std::make_pair(host, port));
  }

  if (repoConf.get<std::string>("storage.method") != "sqlite")
    throw Repo::Error("Only 'sqlite' storage method is supported");

  repoConfig.dbPath = repoConf.get<std::string>("storage.path");

  repoConfig.validatorNode = repoConf.get_child("validator");

  repoConfig.nMaxPackets = repoConf.get<int>("storage.max-packets");

  return repoConfig;
}

Repo::Repo(boost::asio::io_service& ioService, const RepoConfig& config)
  : m_config(config)
  , m_scheduler(ioService)
  , m_face(ioService)
  , m_store(std::make_shared<SqliteStorage>(config.dbPath))
  , m_storageHandle(config.nMaxPackets, *m_store)
  , m_validator(m_face)
  , m_readHandle(m_face, m_storageHandle, m_keyChain, m_scheduler)
  , m_writeHandle(m_face, m_storageHandle, m_keyChain, m_scheduler, m_validator)
  , m_watchHandle(m_face, m_storageHandle, m_keyChain, m_scheduler, m_validator)
  , m_deleteHandle(m_face, m_storageHandle, m_keyChain, m_scheduler, m_validator)
  , m_tcpBulkInsertHandle(ioService, m_storageHandle)

{
  m_validator.load(config.validatorNode, config.repoConfigPath);
}

void
Repo::initializeStorage()
{
  // Rebuild storage if storage checkpoin exists
  ndn::time::steady_clock::TimePoint start = ndn::time::steady_clock::now();
  m_storageHandle.initialize();
  ndn::time::steady_clock::TimePoint end = ndn::time::steady_clock::now();
  ndn::time::milliseconds cost = ndn::time::duration_cast<ndn::time::milliseconds>(end - start);
  std::cerr << "initialize storage cost: " << cost << "ms" << std::endl;
}

void
Repo::enableListening()
{
  // Enable "listening" on Data prefixes
  for (auto it = m_config.dataPrefixes.begin();
       it != m_config.dataPrefixes.end();
       ++it)
    {
      m_readHandle.listen(*it);
    }

  // Enable "listening" on control prefixes
  for (auto it = m_config.repoPrefixes.begin();
       it != m_config.repoPrefixes.end();
       ++it)
    {
      m_writeHandle.listen(*it);
      m_watchHandle.listen(*it);
      m_deleteHandle.listen(*it);
    }

  // Enable listening on TCP bulk insert addresses
  for (auto it = m_config.tcpBulkInsertEndpoints.begin();
       it != m_config.tcpBulkInsertEndpoints.end();
       ++it)
    {
      m_tcpBulkInsertHandle.listen(it->first, it->second);
    }
}

void
Repo::enableValidation()
{
  m_validator.load(m_config.validatorNode, m_config.repoConfigPath);
}

} // namespace repo
