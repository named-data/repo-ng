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

#include "../../build/src/config.hpp"
#include "sqlite-storage.hpp"
#include "index.hpp"
#include <boost/filesystem.hpp>
#include <istream>

namespace repo {

using std::string;

SqliteStorage::SqliteStorage(const string& dbPath)
  : m_size(0)
{
  if (dbPath.empty()) {
    std::cerr << "Create db file in local location [" << dbPath << "]. " << std::endl
              << "You can assign the path using -d option" << std::endl;
    m_dbPath = string("ndn_repo.db");
  }
  else {
    boost::filesystem::path fsPath(dbPath);
    boost::filesystem::file_status fsPathStatus = boost::filesystem::status(fsPath);
    if (!boost::filesystem::is_directory(fsPathStatus)) {
      if (!boost::filesystem::create_directory(boost::filesystem::path(fsPath))) {
        throw Error("Folder '" + dbPath + "' does not exists and cannot be created");
      }
    }

    m_dbPath = dbPath + "/ndn_repo.db";
  }
  initializeRepo();
}


void
SqliteStorage::initializeRepo()
{
  char* errMsg = 0;

  int rc = sqlite3_open_v2(m_dbPath.c_str(), &m_db,
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
#ifdef DISABLE_SQLITE3_FS_LOCKING
                           "unix-dotfile"
#else
                           0
#endif
                           );

  if (rc == SQLITE_OK) {
    sqlite3_exec(m_db, "CREATE TABLE NDN_REPO ("
                      "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                      "name BLOB, "
                      "data BLOB, "
                      "keylocatorHash BLOB);\n "
                 , 0, 0, &errMsg);
    // Ignore errors (when database already exists, errors are expected)
  }
  else {
    std::cerr << "Database file open failure rc:" << rc << std::endl;
    throw Error("Database file open failure");
  }
  sqlite3_exec(m_db, "PRAGMA synchronous = OFF", 0, 0, &errMsg);
  sqlite3_exec(m_db, "PRAGMA journal_mode = WAL", 0, 0, &errMsg);
}

SqliteStorage::~SqliteStorage()
{
  sqlite3_close(m_db);
}

void
SqliteStorage::fullEnumerate(const ndn::function
                             <void(const Storage::ItemMeta)>& f)
{
  sqlite3_stmt* m_stmt = 0;
  int rc = SQLITE_DONE;
  string sql = string("SELECT id, name, keylocatorHash FROM NDN_REPO;");
  rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_stmt, 0);
  if (rc != SQLITE_OK)
    throw Error("Initiation Read Entries from Database Prepare error");
  int entryNumber = 0;
  while (true) {
    rc = sqlite3_step(m_stmt);
    if (rc == SQLITE_ROW) {

      ItemMeta item;
      item.fullName.wireDecode(Block(sqlite3_column_blob(m_stmt, 1),
                                     sqlite3_column_bytes(m_stmt, 1)));
      item.id = sqlite3_column_int(m_stmt, 0);
      item.keyLocatorHash = make_shared<const ndn::Buffer>
        (ndn::Buffer(sqlite3_column_blob(m_stmt, 3), sqlite3_column_bytes(m_stmt, 3)));

      try {
        f(item);
      }
      catch (...){
        sqlite3_finalize(m_stmt);
        throw;
      }
      entryNumber++;
    }
    else if (rc == SQLITE_DONE) {
      sqlite3_finalize(m_stmt);
      break;
    }
    else {
      std::cerr << "Initiation Read Entries rc:" << rc << std::endl;
      sqlite3_finalize(m_stmt);
      throw Error("Initiation Read Entries error");
    }
  }
  m_size = entryNumber;
}

int64_t
SqliteStorage::insert(const Data& data)
{
  Name name = data.getName();

  Index::Entry entry(data, 0); //the id is not used
  int64_t id = -1;
  if (name.empty()) {
    std::cerr << "name is empty" << std::endl;
    return -1;
  }

  int rc = 0;

  sqlite3_stmt* insertStmt = 0;

  string insertSql = string("INSERT INTO NDN_REPO (id, name, data, keylocatorHash) "
                            "VALUES (?, ?, ?, ?)");

  if (sqlite3_prepare_v2(m_db, insertSql.c_str(), -1, &insertStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(insertStmt);
    std::cerr << "insert sql not prepared" << std::endl;
  }
  //Insert
  if (sqlite3_bind_null(insertStmt, 1) == SQLITE_OK &&
      sqlite3_bind_blob(insertStmt, 2,
                        entry.getName().wireEncode().wire(),
                        entry.getName().wireEncode().size(), 0) == SQLITE_OK &&
      sqlite3_bind_blob(insertStmt, 3,
                        data.wireEncode().wire(),
                        data.wireEncode().size(),0 ) == SQLITE_OK &&
      sqlite3_bind_blob(insertStmt, 4,
                        (const void*)&(*entry.getKeyLocatorHash()),
                        ndn::crypto::SHA256_DIGEST_SIZE,0) == SQLITE_OK) {
    rc = sqlite3_step(insertStmt);
    if (rc == SQLITE_CONSTRAINT) {
      std::cerr << "Insert  failed" << std::endl;
      sqlite3_finalize(insertStmt);
      throw Error("Insert failed");
     }
    sqlite3_reset(insertStmt);
     m_size++;
     id = sqlite3_last_insert_rowid(m_db);
  }
  else {
    throw Error("Some error with insert");
  }

  sqlite3_finalize(insertStmt);
  return id;
}


bool
SqliteStorage::erase(const int64_t id)
{
  sqlite3_stmt* deleteStmt = 0;

  string deleteSql = string("DELETE from NDN_REPO where id = ?;");

  if (sqlite3_prepare_v2(m_db, deleteSql.c_str(), -1, &deleteStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(deleteStmt);
    std::cerr << "delete statement prepared failed" << std::endl;
    throw Error("delete statement prepared failed");
  }

  if (sqlite3_bind_int64(deleteStmt, 1, id) == SQLITE_OK) {
    int rc = sqlite3_step(deleteStmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
      std::cerr << " node delete error rc:" << rc << std::endl;
      sqlite3_finalize(deleteStmt);
      throw Error(" node delete error");
    }
    if (sqlite3_changes(m_db) != 1)
      return false;
    m_size--;
  }
  else {
    std::cerr << "delete bind error" << std::endl;
    sqlite3_finalize(deleteStmt);
    throw Error("delete bind error");
  }
  sqlite3_finalize(deleteStmt);
  return true;
}


shared_ptr<Data>
SqliteStorage::read(const int64_t id)
{
  sqlite3_stmt* queryStmt = 0;
  string sql = string("SELECT * FROM NDN_REPO WHERE id = ? ;");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_int64(queryStmt, 1, id) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        shared_ptr<Data> data(new Data());
        data->wireDecode(Block(sqlite3_column_blob(queryStmt, 2),
                              sqlite3_column_bytes(queryStmt, 2)));
        sqlite3_finalize(queryStmt);
        return data;
      }
      else if (rc == SQLITE_DONE) {
        return shared_ptr<Data>();
      }
      else {
        std::cerr << "Database query failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        throw Error("Database query failure");
      }
    }
    else {
      std::cerr << "select bind error" << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("select bind error");
    }
    sqlite3_finalize(queryStmt);
  }
  else {
    sqlite3_finalize(queryStmt);
    std::cerr << "select statement prepared failed" << std::endl;
    throw Error("select statement prepared failed");
  }
  return shared_ptr<Data>();
}

int64_t
SqliteStorage::size()
{
  sqlite3_stmt* queryStmt = 0;
  string sql("SELECT count(*) FROM NDN_REPO ");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc != SQLITE_OK)
    {
      std::cerr << "Database query failure rc:" << rc << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("Database query failure");
    }

  rc = sqlite3_step(queryStmt);
  if (rc != SQLITE_ROW)
    {
      std::cerr << "Database query failure rc:" << rc << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("Database query failure");
    }

  int64_t nDatas = sqlite3_column_int64(queryStmt, 0);
  if (m_size != nDatas) {
    std::cerr << "The size of database is not correct! " << std::endl;
  }
  return nDatas;
}

} //namespace repo
