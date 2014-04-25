/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_SERVER_REPO_HPP
#define REPO_SERVER_REPO_HPP

#include "../storage/storage-handle.hpp"
#include "../storage/sqlite/sqlite-handle.hpp"
#include "../ndn-handle/read-handle.hpp"
#include "../ndn-handle/write-handle.hpp"
#include "../ndn-handle/delete-handle.hpp"
#include "../ndn-handle/tcp-bulk-insert-handle.hpp"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/util/command-interest-validator.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace repo {

using std::string;
using std::vector;
using std::pair;

struct RepoConfig
{
  //StorageMethod storageMethod; This will be implemtented if there is other method.
  std::string dbPath;
  vector<ndn::Name> dataPrefixes;
  vector<ndn::Name> repoPrefixes;
  vector<pair<string, string> > tcpBulkInsertEndpoints;

  //@todo validator should be configured in config file
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

  void
  enableListening();

private:
  static shared_ptr<StorageHandle>
  openStorage(const RepoConfig& config);

private:
  RepoConfig m_config;
  ndn::Scheduler m_scheduler;
  ndn::Face m_face;
  shared_ptr<StorageHandle> m_storageHandle;
  KeyChain m_keyChain;
  CommandInterestValidator m_validator;
  ReadHandle m_readHandle;
  WriteHandle m_writeHandle;
  DeleteHandle m_deleteHandle;
  TcpBulkInsertHandle m_tcpBulkInsertHandle;
};

} // namespace repo

#endif // REPO_SERVER_REPO_HPP
