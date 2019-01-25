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

#include "repo.hpp"
#include "storage/sqlite-storage.hpp"

#include <ndn-cxx/util/logger.hpp>

namespace repo {

NDN_LOG_INIT(repo.Repo);

RepoConfig
parseConfig(const std::string& configPath)
{
  if (configPath.empty()) {
    NDN_LOG_DEBUG("configuration file path is empty");
  }

  std::ifstream fin(configPath.c_str());
 if (!fin.is_open())
    BOOST_THROW_EXCEPTION(Repo::Error("failed to open configuration file '" + configPath + "'"));

  using namespace boost::property_tree;
  ptree propertyTree;
  try {
    read_info(fin, propertyTree);
  }
  catch (const ptree_error& e) {
    BOOST_THROW_EXCEPTION(Repo::Error("failed to read configuration file '" + configPath + "'"));
  }

  ptree repoConf = propertyTree.get_child("repo");

  RepoConfig repoConfig;
  repoConfig.repoConfigPath = configPath;

  ptree dataConf = repoConf.get_child("data");
  for (const auto& section : dataConf) {
    if (section.first == "prefix")
      repoConfig.dataPrefixes.push_back(Name(section.second.get_value<std::string>()));
    else if (section.first == "registration-subset")
      repoConfig.registrationSubset = section.second.get_value<int>();
    else
      BOOST_THROW_EXCEPTION(Repo::Error("Unrecognized '" + section.first + "' option in 'data' section in "
                                        "configuration file '"+ configPath +"'"));
  }

  ptree commandConf = repoConf.get_child("command");
  for (const auto& section : commandConf) {
    if (section.first == "prefix")
      repoConfig.repoPrefixes.push_back(Name(section.second.get_value<std::string>()));
    else
      BOOST_THROW_EXCEPTION(Repo::Error("Unrecognized '" + section.first + "' option in 'command' section in "
                                        "configuration file '"+ configPath +"'"));
  }

  auto tcpBulkInsert = repoConf.get_child_optional("tcp_bulk_insert");
  bool isTcpBulkEnabled = false;
  std::string host = "localhost";
  std::string port = "7376";
  if (tcpBulkInsert) {
    for (const auto& section : *tcpBulkInsert) {
      isTcpBulkEnabled = true;
      if (section.first == "host") {
        host = section.second.get_value<std::string>();
      }
      else if (section.first == "port") {
        port = section.second.get_value<std::string>();
      }
      else
        BOOST_THROW_EXCEPTION(Repo::Error("Unrecognized '" + section.first + "' option in 'tcp_bulk_insert' section in "
                                          "configuration file '"+ configPath +"'"));
    }
  }
  if (isTcpBulkEnabled) {
    repoConfig.tcpBulkInsertEndpoints.push_back(std::make_pair(host, port));
  }

  if (repoConf.get<std::string>("storage.method") != "sqlite") {
    BOOST_THROW_EXCEPTION(Repo::Error("Only 'sqlite' storage method is supported"));
  }

  repoConfig.dbPath = repoConf.get<std::string>("storage.path");

  repoConfig.validatorNode = repoConf.get_child("validator");

  repoConfig.nMaxPackets = repoConf.get<uint64_t>("storage.max-packets");

  return repoConfig;
}

Repo::Repo(boost::asio::io_service& ioService, const RepoConfig& config)
  : m_config(config)
  , m_scheduler(ioService)
  , m_face(ioService)
  , m_dispatcher(m_face, m_keyChain)
  , m_store(std::make_shared<SqliteStorage>(config.dbPath))
  , m_storageHandle(*m_store)
  , m_validator(m_face)
  , m_readHandle(m_face, m_storageHandle, m_config.registrationSubset)
  , m_writeHandle(m_face, m_storageHandle, m_dispatcher, m_scheduler, m_validator)
  , m_deleteHandle(m_face, m_storageHandle, m_dispatcher, m_scheduler, m_validator)
  , m_tcpBulkInsertHandle(ioService, m_storageHandle)
{
  this->enableValidation();
}

void
Repo::initializeStorage()
{
  // Rebuild storage if storage checkpoin exists
  ndn::time::steady_clock::TimePoint start = ndn::time::steady_clock::now();
  ndn::time::steady_clock::TimePoint end = ndn::time::steady_clock::now();
  ndn::time::milliseconds cost = ndn::time::duration_cast<ndn::time::milliseconds>(end - start);
  NDN_LOG_DEBUG("initialize storage cost: " << cost << "ms");
}

void
Repo::enableListening()
{
  for (const ndn::Name& dataPrefix : m_config.dataPrefixes) {
    // ReadHandle performs prefix registration internally.
    m_readHandle.listen(dataPrefix);
  }
  for (const ndn::Name& cmdPrefix : m_config.repoPrefixes) {
    m_face.registerPrefix(cmdPrefix, nullptr,
      [] (const Name& cmdPrefix, const std::string& reason) {
        NDN_LOG_DEBUG("Command prefix " << cmdPrefix << " registration error: " << reason);
        BOOST_THROW_EXCEPTION(Error("Command prefix registration failed"));
      });

    m_dispatcher.addTopPrefix(cmdPrefix);
  }

  for (const auto& ep : m_config.tcpBulkInsertEndpoints) {
    m_tcpBulkInsertHandle.listen(ep.first, ep.second);
  }
}

void
Repo::enableValidation()
{
  m_validator.load(m_config.validatorNode, m_config.repoConfigPath);
}

} // namespace repo
