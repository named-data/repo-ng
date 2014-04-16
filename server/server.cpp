/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include <string>
#include <iostream>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/util/command-interest-validator.hpp>

#include "../storage/storage-handle.hpp"
#include "../storage/sqlite/sqlite-handle.hpp"
#include "../ndn-handle/read-handle.hpp"
#include "../ndn-handle/write-handle.hpp"
#include "../ndn-handle/tcp-bulk-insert-handle.hpp"
#include "../ndn-handle/delete-handle.hpp"

using namespace repo;

static const string ndnRepoUsageMessage =
  "ndn-repo - NDNx Repository Daemon\n"
  "-d: set database path\n"
  "-h: show help message\n"
  "-c: set config file path\n"
  ;

int
main(int argc, char** argv) {
  int opt;
  string dbPath;
  string confPath;
  while ((opt = getopt(argc, argv, "d:hc:")) != -1) {
    switch (opt) {
    case 'd':
      dbPath = string(optarg);
      break;
    case 'h':
      std::cout << ndnRepoUsageMessage << std::endl;
      return 1;
    case 'c':
      confPath = string(optarg);
      break;
    default:
      break;
    }
  }

  if (confPath.empty()) {
    confPath = "./repo.conf";
  }

  Name dataPrefix("ndn:/");
  Name repoPrefix("ndn:/example/repo");
  /// @todo read from configuration

  SqliteHandle sqliteHandle(dbPath);

  shared_ptr<boost::asio::io_service> io =
    ndn::make_shared<boost::asio::io_service>();

  Face face(io);
  Scheduler scheduler(*io);

  /// @todo specify trust model
  CommandInterestValidator validator;
  KeyChain keyChain;

  ReadHandle readHandle(face, sqliteHandle, keyChain, scheduler);
  readHandle.listen(dataPrefix);

  WriteHandle writeHandle(face, sqliteHandle, keyChain, scheduler, validator);
  writeHandle.listen(repoPrefix);

  DeleteHandle deleteHandle(face, sqliteHandle, keyChain, scheduler, validator);
  deleteHandle.listen(repoPrefix);

  TcpBulkInsertHandle tcpBulkInsertHandle(*io, sqliteHandle);
  tcpBulkInsertHandle.listen("localhost", "7376");

  face.processEvents();
  return 0;
}
