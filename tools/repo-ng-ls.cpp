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

#include "../src/common.hpp"
#include "config.hpp"

#include <iostream>
#include <string>

#include <ndn-cxx/util/sqlite3-statement.hpp>

#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <sqlite3.h>

namespace repo {

static void
printUsage(const char* programName)
{
  std::cout
    << "Usage:\n"
    << "  " << programName << " [-c <path/to/repo-ng.conf>] [-n] [-h]\n"
    << "\n"
    << "List names of Data packets in NDN repository. "
    << "By default, all names will include the implicit digest of Data packets\n"
    << "\n"
    << "Options:\n"
    << "  -h: show help message\n"
    << "  -c: set config file path\n"
    << "  -n: do not show implicit digest\n"
    << std::endl;
}

class RepoEnumerator
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
  RepoEnumerator(const std::string& configFile);

  uint64_t
  enumerate(bool showImplicitDigest);

private:
  void
  readConfig(const std::string& configFile);

private:
  sqlite3* m_db;
  std::string m_dbPath;
};

RepoEnumerator::RepoEnumerator(const std::string& configFile)
{
  readConfig(configFile);
  char* errMsg = nullptr;
  int rc = sqlite3_open_v2(m_dbPath.c_str(), &m_db, SQLITE_OPEN_READONLY,
   #ifdef DISABLE_SQLITE3_FS_LOCKING
                           "unix-dotfile"
   #else
                           nullptr
   #endif
                          );
  if (rc != SQLITE_OK) {
    BOOST_THROW_EXCEPTION(Error("Database file open failure"));
  }
  sqlite3_exec(m_db, "PRAGMA synchronous = OFF", nullptr, nullptr, &errMsg);
  sqlite3_exec(m_db, "PRAGMA journal_mode = WAL", nullptr, nullptr, &errMsg);
}

void
RepoEnumerator::readConfig(const std::string& configFile)
{
  if (configFile.empty()) {
    BOOST_THROW_EXCEPTION(Error("Invalid configuration file name"));
  }

  std::ifstream fin(configFile.c_str());
  if (!fin.is_open())
    BOOST_THROW_EXCEPTION(Error("failed to open configuration file '" + configFile + "'"));

  using namespace boost::property_tree;
  ptree propertyTree;
  try {
    read_info(fin, propertyTree);
  }
  catch (const ptree_error& e) {
    BOOST_THROW_EXCEPTION(Error("failed to read configuration file '" + configFile + "'"));
  }
  ptree repoConf = propertyTree.get_child("repo");
  m_dbPath = repoConf.get<std::string>("storage.path");
  m_dbPath += "/ndn_repo.db";
}

uint64_t
RepoEnumerator::enumerate(bool showImplicitDigest)
{
  ndn::util::Sqlite3Statement stmt(m_db, "SELECT data FROM NDN_REPO_V2;");
  uint64_t nEntries = 0;
  while (true) {
    int rc = stmt.step();
    if (rc == SQLITE_ROW) {
      Data data(stmt.getBlock(0));
      if (showImplicitDigest) {
        std::cout << data.getFullName() << std::endl;
      }
      else {
        std::cout << data.getName() << std::endl;
      }
      nEntries++;
    }
    else if (rc == SQLITE_DONE) {
      break;
    }
    else {
      BOOST_THROW_EXCEPTION(Error("Initiation Read Entries error"));
    }
  }
  return nEntries;
}

int
main(int argc, char** argv)
{
  std::string configPath = DEFAULT_CONFIG_FILE;
  bool showImplicitDigest = true;
  int opt;
  while ((opt = getopt(argc, argv, "hc:n")) != -1) {
    switch (opt) {
    case 'h':
      printUsage(argv[0]);
      return 0;
    case 'c':
      configPath = std::string(optarg);
      break;
    case 'n':
      showImplicitDigest = false;
      break;
    default:
      break;
    }
  }

  RepoEnumerator instance(configPath);
  uint64_t count = instance.enumerate(showImplicitDigest);
  std::cerr << "Total number of data = " << count << std::endl;
  return 0;
}

} // namespace repo


int
main(int argc, char** argv)
{
  try {
    return repo::main(argc, argv);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
}
