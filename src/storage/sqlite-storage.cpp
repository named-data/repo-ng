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

#include "sqlite-storage.hpp"
#include "config.hpp"

#include <ndn-cxx/util/sha256.hpp>
#include <ndn-cxx/util/sqlite3-statement.hpp>

#include <boost/filesystem.hpp>
#include <istream>

#include <ndn-cxx/util/logger.hpp>

namespace repo {

NDN_LOG_INIT(repo.SqliteStorage);

SqliteStorage::SqliteStorage(const std::string& dbPath)
{
  if (dbPath.empty()) {
    m_dbPath = std::string("ndn_repo.db");
    NDN_LOG_DEBUG("Create db file in local location [" << m_dbPath << "]. " );
    NDN_LOG_DEBUG("You can assign the path using -d option" );
  }
  else {
    boost::filesystem::path fsPath(dbPath);
    boost::filesystem::file_status fsPathStatus = boost::filesystem::status(fsPath);
    if (!boost::filesystem::is_directory(fsPathStatus)) {
      if (!boost::filesystem::create_directory(boost::filesystem::path(fsPath))) {
        NDN_THROW(Error("Folder '" + dbPath + "' does not exists and cannot be created"));
      }
    }

    m_dbPath = dbPath + "/ndn_repo.db";
  }
  initializeRepo();
}


void
SqliteStorage::initializeRepo()
{
  char* errMsg = nullptr;
  int rc = sqlite3_open_v2(m_dbPath.c_str(), &m_db,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
#ifdef DISABLE_SQLITE3_FS_LOCKING
                           "unix-dotfile"
#else
                           nullptr
#endif
                          );

  if (rc == SQLITE_OK) {
    // Create a new table named NDN_REPO_V2, distinguish from the old table name(NDN_REPO)
    sqlite3_exec(m_db, "CREATE TABLE NDN_REPO_V2 (name BLOB, data BLOB);", nullptr, nullptr, &errMsg);
    // Ignore errors (when database already exists, errors are expected)
    sqlite3_exec(m_db, "CREATE UNIQUE INDEX index_name ON NDN_REPO_V2 (name);", nullptr, nullptr, &errMsg);
  }
  else {
    NDN_LOG_DEBUG("Database file open failure rc:" << rc);
    NDN_THROW(Error("Database file open failure"));
  }

  // SQLite continues without syncing as soon as it has handed data off to the operating system
  sqlite3_exec(m_db, "PRAGMA synchronous = OFF;", nullptr, nullptr, &errMsg);
  // Uses a write-ahead log instead of a rollback journal to implement transactions.
  sqlite3_exec(m_db, "PRAGMA journal_mode = WAL;", nullptr, nullptr, &errMsg);
}

SqliteStorage::~SqliteStorage()
{
  sqlite3_close(m_db);
}

int64_t
SqliteStorage::insert(const Data& data)
{
  Name name = data.getFullName(); // store the full name
  ndn::util::Sqlite3Statement stmt(m_db, "INSERT INTO NDN_REPO_V2 (name, data) VALUES (?, ?);");

  //Insert
  // Bind NULL to name value in NDN_REPO_V2 when initialize result.
  auto result = sqlite3_bind_null(stmt, 1);
  if (result == SQLITE_OK) {
    result = stmt.bind(1, name.wireEncode().value(),
                          name.wireEncode().value_size(), SQLITE_STATIC);
  }
  if (result == SQLITE_OK) {
    result = stmt.bind(2, data.wireEncode(), SQLITE_STATIC);
  }

  int id = 0;
  if (result == SQLITE_OK) {
    int rc = 0;
    rc = stmt.step();
    if (rc == SQLITE_CONSTRAINT) {
      NDN_LOG_DEBUG("Insert failed");
      NDN_THROW(Error("Insert failed"));
    }
    sqlite3_reset(stmt);
    id = sqlite3_last_insert_rowid(m_db);
  }
  else {
    NDN_THROW(Error("Some error with insert"));
  }
  return id;
}

bool
SqliteStorage::erase(const Name& name)
{
  ndn::util::Sqlite3Statement stmt(m_db, "DELETE FROM NDN_REPO_V2 WHERE name = ?;");

  auto result = stmt.bind(1,
                          name.wireEncode().value(),
                          name.wireEncode().value_size(), SQLITE_STATIC);

  if (result == SQLITE_OK) {
    int rc = stmt.step();
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
      NDN_LOG_DEBUG("Node delete error rc:" << rc);
      NDN_THROW(Error("Node delete error"));
    }
    if (sqlite3_changes(m_db) != 1) {
      return false;
    }
  }
  else {
    NDN_LOG_DEBUG("delete bind error");
    NDN_THROW(Error("delete bind error"));
  }
  return true;
}

std::shared_ptr<Data>
SqliteStorage::read(const Name& name)
{
  return find(name);
}

bool
SqliteStorage::has(const Name& name)
{
  // find exact match
  return find(name, true) != nullptr;
}

std::shared_ptr<Data>
SqliteStorage::find(const Name& name, bool exactMatch)
{
  NDN_LOG_DEBUG("Trying to find: " << name);
  Name nameSuccessor;
  if (!exactMatch) {
    nameSuccessor = name.getSuccessor();
  }

  std::string sql;
  if (exactMatch)
    sql = "SELECT * FROM NDN_REPO_V2 WHERE name = ?;";
  else
    sql = "SELECT * FROM NDN_REPO_V2 WHERE name >= ? and name < ?;";

  ndn::util::Sqlite3Statement stmt(m_db, sql);

  auto result = stmt.bind(1,
                          name.wireEncode().value(),
                          name.wireEncode().value_size(), SQLITE_STATIC);

  // use getsuccessor to locate prefix match items
  if (result == SQLITE_OK && !exactMatch) {
    // use V in TLV for prefix match when there is no exact match
    result = stmt.bind(2,
                       nameSuccessor.wireEncode().value(),
                       nameSuccessor.wireEncode().value_size(), SQLITE_STATIC);
  }

  if (result == SQLITE_OK) {
    int rc = stmt.step();
    if (rc == SQLITE_ROW) {
      Name foundName;

      auto data = std::make_shared<Data>();
      try {
        data->wireDecode(stmt.getBlock(1));
      }
      catch (const ndn::Block::Error& error) {
        NDN_LOG_DEBUG(error.what());
        return nullptr;
      }
      NDN_LOG_DEBUG("Data from db: " << *data);

      foundName = data->getFullName();

      if ((exactMatch && name == foundName) || (!exactMatch && name.isPrefixOf(foundName))) {
        NDN_LOG_DEBUG("Found: " << foundName << " " << stmt.getInt(0));
        return data;
      }
    }
    else if (rc == SQLITE_DONE) {
      return nullptr;
    }
    else {
      NDN_LOG_DEBUG("Database query failure rc:" << rc);
      NDN_THROW(Error("Database query failure"));
    }
  }
  else {
    NDN_LOG_DEBUG("select bind error");
    NDN_THROW(Error("select bind error"));
  }
  return nullptr;
}

void
SqliteStorage::forEach(const std::function<void(const Name&)>& f)
{
  ndn::util::Sqlite3Statement stmt(m_db, "SELECT data FROM NDN_REPO_V2;");

  while (true) {
    int rc = stmt.step();
    if (rc == SQLITE_ROW) {
      Data data;
      try {
        data.wireDecode(stmt.getBlock(0));
      }
      catch (const ndn::Block::Error& error) {
        NDN_LOG_DEBUG("Error while decoding data from the database: " << error.what());
        continue;
      }
      f(data.getName());
    }
    else if (rc == SQLITE_DONE) {
      break;
    }
    else {
      NDN_THROW(Error("Database query failure (code: " + ndn::to_string(rc)));
    }
  }
}

uint64_t
SqliteStorage::size()
{
  ndn::util::Sqlite3Statement stmt(m_db, "SELECT count(*) FROM NDN_REPO_V2;");

  int rc = stmt.step();
  if (rc != SQLITE_ROW) {
    NDN_LOG_DEBUG("Database query failure rc:" << rc);
    NDN_THROW(Error("Database query failure"));
  }

  uint64_t nData = stmt.getInt(0);
  return nData;
}

} // namespace repo
