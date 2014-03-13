/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include <string>
#include <iostream>
#include <ndn-cpp-dev/face.hpp>

#include "../storage/storage-handle.hpp"
#include "../storage/sqlite/sqlite-handle.hpp"
#include "../ndn-handle/read-handle.hpp"

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
  SqliteHandle sqliteHandle(dbPath);
  StorageHandle* handle = &sqliteHandle;

  Face face;
  ReadHandle readHandle(&face, handle);
  if (confPath.empty()) {
    confPath = "./repo.conf";
  }
  face.processEvents();
  return 0;
}
