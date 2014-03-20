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

#include "config.hpp"
#include "sqlite-handle.hpp"
#include <boost/filesystem.hpp>

namespace repo {

SqliteHandle::SqliteHandle(const string& dbPath)
  : StorageHandle(STORAGE_METHOD_SQLITE)
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
SqliteHandle::initializeRepo()
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
                      "name BLOB PRIMARY KEY, "
                      "data BLOB, "
                      "parentName BLOB, "
                      "nChildren INTEGER);\n"
                      "CREATE INDEX NdnRepoParentName ON NDN_REPO (parentName);\n"
                      "CREATE INDEX NdnRepoData ON NDN_REPO (data);\n"
                 , 0, 0, &errMsg);
    // Ignore errors (when database already exists, errors are expected)
  }
  else {
    std::cerr << "Database file open failure rc:" << rc << std::endl;
    throw Error("Database file open failure");
  }

  Name rootName;
  string sql = string("SELECT * FROM NDN_REPO WHERE name = ?;");

  sqlite3_stmt* queryStmt = 0;

  rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_blob(queryStmt, 1, rootName.wireEncode().wire(),
                          rootName.wireEncode().size(), 0) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        std::cerr << "root has been created" << std::endl;
      }
      else if (rc == SQLITE_DONE) {
        sqlite3_stmt* p2Stmt = 0;
        sql = string("INSERT INTO NDN_REPO (name, data, parentName, nChildren) "
                     " VALUES (?,    ?,    ?,     ?);");
        rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &p2Stmt, 0);
        if (rc == SQLITE_OK) {
          if (sqlite3_bind_blob(p2Stmt, 1, rootName.wireEncode().wire(),
                                rootName.wireEncode().size(), 0) == SQLITE_OK &&
              sqlite3_bind_null(p2Stmt, 2) == SQLITE_OK &&
              sqlite3_bind_null(p2Stmt, 3) == SQLITE_OK &&
              sqlite3_bind_int(p2Stmt, 4, 0) == SQLITE_OK) {
            rc = sqlite3_step(p2Stmt);;
            if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
              std::cerr << "Root name insert failure rc:" << rc << std::endl;
              sqlite3_finalize(p2Stmt);
              throw Error("Root name insert failure");
            }
          }
          else {
            std::cerr << "bind blob failure rc:" << rc << std::endl;
            sqlite3_finalize(p2Stmt);
            throw Error("bind blob failure");
          }
        }
        else {
          std::cerr << "p2Stmt prepared rc:" << rc << std::endl;
          sqlite3_finalize(p2Stmt);
          throw Error("p2Stmt prepared");
        }
        sqlite3_finalize(p2Stmt);
      }
      else {
        std::cerr << "Database query failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        throw Error("Database query failure");
      }
    }
    sqlite3_finalize(queryStmt);
  }
  sqlite3_exec(m_db, "PRAGMA synchronous = OFF", 0, 0, &errMsg);
  sqlite3_exec(m_db, "PRAGMA journal_mode = WAL", 0, 0, &errMsg);
}

SqliteHandle::~SqliteHandle()
{
  sqlite3_close(m_db);
}


//Temporarily assigned the datatype of every component. needs to be further discussed

bool
SqliteHandle::insertData(const Data& data)
{
  Name name = data.getName();

  if (name.empty()) {
    std::cerr << "name is empty" << std::endl;
    return false;
  }

  int rc = 0;

  string updateSql2 = string("UPDATE NDN_REPO SET data = ? WHERE name = ?;");
  //std::cerr << "update" << std::endl;
  sqlite3_stmt* update2Stmt = 0;
  if (sqlite3_prepare_v2(m_db, updateSql2.c_str(), -1, &update2Stmt, 0) != SQLITE_OK) {
    sqlite3_finalize(update2Stmt);
    std::cerr << "update sql2 not prepared" << std::endl;
    throw Error("update sql2 not prepared");
  }
  if (sqlite3_bind_blob(update2Stmt, 1,
                        data.wireEncode().wire(),
                        data.wireEncode().size(), 0) == SQLITE_OK &&
      sqlite3_bind_blob(update2Stmt, 2,
                        name.wireEncode().wire(),
                        name.wireEncode().size(), 0) == SQLITE_OK) {
    rc = sqlite3_step(update2Stmt);
    sqlite3_finalize(update2Stmt);
    if (rc != SQLITE_DONE) {
      return false;
    }
    //what error??
    //std::cerr << "update rc:" << rc << std::endl;
    /// \todo Do something with rc
  }
  int changeCount = sqlite3_changes(m_db);
  //std::cerr << "changeCount: " << changeCount << std::endl;
  if (changeCount > 0) {
    return true;
  }

  sqlite3_stmt* insertStmt = 0;
  sqlite3_stmt* updateStmt = 0;
  string insertSql = string("INSERT INTO NDN_REPO (name, data, parentName, nChildren) "
                            "VALUES (?, ?, ?, ?)");
  string updateSql = string("UPDATE NDN_REPO SET nChildren = nChildren + 1 WHERE name = ?");

  Name rootName;


  if (sqlite3_prepare_v2(m_db, insertSql.c_str(), -1, &insertStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(insertStmt);
    std::cerr << "insert sql not prepared" << std::endl;
  }
  if (sqlite3_prepare_v2(m_db, updateSql.c_str(), -1, &updateStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(updateStmt);
    std::cerr << "update sql not prepared" << std::endl;
    throw Error("update sql not prepared");
  }

  //Insert and read the prefix
  Name parentName = name;
  Name grandName;
  do {
    parentName = parentName.getPrefix(-1);
    if (!hasName(parentName)) {
      grandName = parentName.getPrefix(-1);
      if (sqlite3_bind_blob(insertStmt, 1,
                            parentName.wireEncode().wire(),
                            parentName.wireEncode().size(), 0) == SQLITE_OK &&
          sqlite3_bind_null(insertStmt, 2) == SQLITE_OK &&
          sqlite3_bind_blob(insertStmt, 3,
                            grandName.wireEncode().wire(),
                            grandName.wireEncode().size(), 0) == SQLITE_OK &&
          sqlite3_bind_int(insertStmt, 4, 1) == SQLITE_OK) {
        rc = sqlite3_step(insertStmt);
        if (rc == SQLITE_CONSTRAINT) {
          std::cerr << "Insert parent prefix failed" << std::endl;
          sqlite3_finalize(insertStmt);
          throw Error("Insert parent prefix failed");
        }
        sqlite3_reset(insertStmt);
      }
    }
    else {
      break;
    }
  } while (!parentName.empty());

  //The existed parent nChildren + 1

  if (sqlite3_bind_blob(updateStmt, 1, parentName.wireEncode().wire(),
                        parentName.wireEncode().size(), 0) == SQLITE_OK) {
    rc = sqlite3_step(updateStmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
      std::cerr << "update error rc:" << rc << std::endl;
      sqlite3_finalize(updateStmt);
      sqlite3_finalize(insertStmt);
      throw Error("update error");
    }
    sqlite3_reset(updateStmt);
  }

  //Insert the name and the data, if this data name exists update, else insert data

  parentName = name.getPrefix(-1);
  sqlite3_reset(insertStmt);
  if (sqlite3_bind_blob(insertStmt, 1,
                        name.wireEncode().wire(),
                        name.wireEncode().size(), 0) == SQLITE_OK &&
      sqlite3_bind_blob(insertStmt, 2,
                        data.wireEncode().wire(),
                        data.wireEncode().size(), 0) == SQLITE_OK &&
      sqlite3_bind_blob(insertStmt, 3,
                        parentName.wireEncode().wire(),
                        parentName.wireEncode().size(), 0) == SQLITE_OK &&
      sqlite3_bind_int(insertStmt, 4, 0) == SQLITE_OK) {
    rc = sqlite3_step(insertStmt);
    //std::cerr << "insert rc:" << rc << std::endl;
    //std::cerr << "insert the data: " << data.wireEncode().wire() << std::endl;
    if (rc == SQLITE_CONSTRAINT) {
      std::cerr << "The name of the data has existed!" << std::endl;
      sqlite3_finalize(insertStmt);
      return false;
    }
  }

  sqlite3_finalize(updateStmt);
  sqlite3_finalize(insertStmt);
  return true;
}

bool
SqliteHandle::deleteData(const Name& name)
{
  sqlite3_stmt* queryStmt = 0;
  sqlite3_stmt* deleteStmt = 0;
  sqlite3_stmt* updateStmt = 0;
  sqlite3_stmt* update2Stmt = 0;

  string querySql = string("SELECT * from NDN_REPO where name = ?;");
  string deleteSql = string("DELETE from NDN_REPO where name = ?;");

  string updateSql = string("UPDATE NDN_REPO SET nChildren = nChildren - 1 WHERE name = ?;");
  string updateSql2 = string("UPDATE NDN_REPO SET data = NULL WHERE name = ?;");

  int rc = sqlite3_prepare_v2(m_db, querySql.c_str(), -1, &queryStmt, 0);
  Name tmpName = name;
  int nChildren = -1;
  if (sqlite3_prepare_v2(m_db, deleteSql.c_str(), -1, &deleteStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(deleteStmt);
    std::cerr << "delete statement prepared failed" << std::endl;
    throw Error("delete statement prepared failed");
  }
  if (sqlite3_prepare_v2(m_db, updateSql.c_str(), -1, &updateStmt, 0) != SQLITE_OK) {
    sqlite3_finalize(updateStmt);
    std::cerr << "delete update prepared failed" << std::endl;
    throw Error("delete update prepared failed");
  }
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_blob(queryStmt, 1,
                          tmpName.wireEncode().wire(),
                          tmpName.wireEncode().size(), 0) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        nChildren = sqlite3_column_int(queryStmt, 3);
      }
      else {
        std::cerr << "Database query no such name or failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        return false;
      }
    }
    if (nChildren > 0) {
      //update internal node, so just update and return
      if (sqlite3_prepare_v2(m_db, updateSql2.c_str(), -1, &update2Stmt, 0) != SQLITE_OK) {
        sqlite3_finalize(update2Stmt);
        std::cerr << "delete update prepared failed" << std::endl;
        throw Error("delete update prepared failed");
      }
      if (sqlite3_bind_blob(update2Stmt, 1,
                            tmpName.wireEncode().wire(),
                            tmpName.wireEncode().size(), 0) == SQLITE_OK) {
        rc = sqlite3_step(update2Stmt);
        std::cerr << "deleteData update" << std::endl;
      }
      else {
        std::cerr << "delete bind error" << std::endl;
        sqlite3_finalize(update2Stmt);
        throw Error("delete bind error");
      }
      return true;
    }
    else {
      //Delete the leaf node
      if (sqlite3_bind_blob(deleteStmt, 1,
                            tmpName.wireEncode().wire(),
                            tmpName.wireEncode().size(), 0) == SQLITE_OK) {
        rc = sqlite3_step(deleteStmt);
        if (rc != SQLITE_DONE && rc !=SQLITE_ROW) {
          std::cerr << "leaf node delete error rc:" << rc << std::endl;
          sqlite3_finalize(deleteStmt);
          throw Error("leaf node delete error");
        }
      }
      else {
        std::cerr << "delete bind error" << std::endl;
        sqlite3_finalize(deleteStmt);
        throw Error("delete bind error");
      }
      sqlite3_reset(deleteStmt);
    }
    queryStmt = 0;
    rc = sqlite3_prepare_v2(m_db, querySql.c_str(), -1, &queryStmt, 0);
    if (rc != SQLITE_OK) {
      std::cerr << "prepare error" << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("prepare error");
    }
    //read prefix if nChildren is 0 and data is 0
    int dataSize = 0;
    do {
      tmpName = tmpName.getPrefix(-1);
      if (sqlite3_bind_blob(queryStmt, 1,
                            tmpName.wireEncode().wire(),
                            tmpName.wireEncode().size(), 0) == SQLITE_OK) {
        rc = sqlite3_step(queryStmt);
        if (rc == SQLITE_ROW) {
          nChildren = sqlite3_column_int(queryStmt, 3);
          dataSize = sqlite3_column_bytes(queryStmt, 1);
        }
        else {
          std::cerr << "Database query no such name or failure rc:" << rc << std::endl;
          sqlite3_finalize(queryStmt);
          return false;
        }
        if (nChildren == 1 && !tmpName.empty() && dataSize == 0) {
          //Delete this internal node
          if (sqlite3_bind_blob(deleteStmt, 1,
                                tmpName.wireEncode().wire(),
                                tmpName.wireEncode().size(), 0) == SQLITE_OK) {
            rc = sqlite3_step(deleteStmt);
            if (rc != SQLITE_DONE && rc !=SQLITE_ROW) {
              std::cerr << "internal node delete error rc:" << rc << std::endl;
              sqlite3_finalize(deleteStmt);
              throw Error("internal node delete error");
            }
          }
          else {
            std::cerr << "delete bind error" << std::endl;
            sqlite3_finalize(deleteStmt);
            throw Error("delete bind error");
          }
          sqlite3_reset(deleteStmt);
        }
        else {
          //nChildren - 1
          if (sqlite3_bind_blob(updateStmt, 1,
                                tmpName.wireEncode().wire(),
                                tmpName.wireEncode().size(), 0) == SQLITE_OK) {
            rc = sqlite3_step(updateStmt);
            if (rc != SQLITE_DONE && rc !=SQLITE_ROW) {
              std::cerr << "internal node nChildren update error rc:" << rc << std::endl;
              sqlite3_finalize(updateStmt);
              throw Error("internal node nChildren update error");
            }
          }
          else {
            std::cerr << "update bind error" << std::endl;
            sqlite3_finalize(updateStmt);
            throw Error("update bind error");
          }
          sqlite3_reset(updateStmt);
          break;
        }
      }
      else {
        std::cerr << "query bind error" << std::endl;
        sqlite3_finalize(queryStmt);
        throw Error("query bind error");
      }
      sqlite3_reset(queryStmt);
    } while (!tmpName.empty());

  }
  else {
    std::cerr << "query prepared failure rc:" << rc << std::endl;
    sqlite3_finalize(queryStmt);
    throw Error("query prepared failure");
  }
  return true;
}

bool
SqliteHandle::readData(const Interest& interest, Data& data)
{
  vector<Name> names;
  Name resultName;
  if (!interest.hasSelectors()) {
    return readDataPlain(interest.getName(), data);
  }
  else {
    if (readNameSelector(interest, names)) {
      if (names.empty())
        return false;
      if (!filterNameChild(interest.getName(), interest.getChildSelector(), names, resultName)) {
        return false;
      }
    }
    return readData(resultName, data);
  }
}

// This function is the first version of data read following longest prefix match.
// It will return the leftmost data
bool
SqliteHandle::readDataPlain(const Name& name, Data& data)
{
  vector<Name> names;
  Name resultName;
  readDataName(name, names);
  if (names.empty())
    return false;
  bool isOk = filterNameChild(name, 0, names, resultName);
  if (isOk) {
    return readData(resultName, data);
  }
  else
  {
    return false;
  }
}

// retrieve all the leaf nodes of a subtree
bool
SqliteHandle::readDataName(const Name& name, vector<Name>& names) const
{
  if (name.empty()) {
    std::cerr << "The name is empty" << std::endl;
    return false;
  }
  Name tmpName = name;
  //This queue is for internal node;
  queue<Name> internalNames;

  // Step 1. Check if the requested name corresponds to a leaf (data is not NULL)
  string sql = string("SELECT * FROM NDN_REPO WHERE name = ? AND data IS NOT NULL;");
  sqlite3_stmt* queryStmt = 0;
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc != SQLITE_OK)
    throw Error("prepare error");

  if (sqlite3_bind_blob(queryStmt, 1,
                        tmpName.wireEncode().wire(),
                        tmpName.wireEncode().size(), 0) == SQLITE_OK) {
    rc = sqlite3_step(queryStmt);
    if (rc == SQLITE_ROW) {
      int nChildren = sqlite3_column_int(queryStmt, 3);
      Name elementName;
      elementName.wireDecode(Block(sqlite3_column_blob(queryStmt, 0),
                                   sqlite3_column_bytes(queryStmt, 0)));
      names.push_back(elementName);
      if (nChildren == 0) {
        sqlite3_finalize(queryStmt);
        return true;
      }
    }
    else if (rc == SQLITE_DONE) {
      // ignore
    }
    else {
      std::cerr << "read error rc:" << rc << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("read error");
    }
  }
  sqlite3_finalize(queryStmt);


  // Step 2. Recursively find all data packets with the specified prefix
  string psql = string("SELECT * FROM NDN_REPO WHERE parentName = ?;");
  sqlite3_stmt* queryParentStmt = 0;
  rc = sqlite3_prepare_v2(m_db, psql.c_str(), -1, &queryParentStmt, 0);
  if (rc != SQLITE_OK)
    throw Error("prepare error");

  internalNames.push(tmpName);
  while (!internalNames.empty()) {
    tmpName = internalNames.front();
    internalNames.pop();
    if (sqlite3_bind_blob(queryParentStmt, 1,
                          tmpName.wireEncode().wire(),
                          tmpName.wireEncode().size(), 0) == SQLITE_OK) {
      while (true) {
        rc = sqlite3_step(queryParentStmt);
        if (rc == SQLITE_ROW) {
          Name elementName;
          elementName.wireDecode(Block(sqlite3_column_blob(queryParentStmt, 0),
                                       sqlite3_column_bytes(queryParentStmt, 0)));
          int nChildren = sqlite3_column_int(queryParentStmt, 3);
          if (nChildren > 0) {
            internalNames.push(elementName);
          }
          if (sqlite3_column_type(queryParentStmt, 1) != SQLITE_NULL) {
            names.push_back(elementName);
          }
        }
        else if (rc == SQLITE_DONE) {
          break;
        }
        else {
          std::cerr << "read error rc:" << rc << std::endl;
          sqlite3_finalize(queryParentStmt);
          throw Error("read error");
        }
      }
      sqlite3_reset(queryParentStmt);
    }
    else {
      std::cerr << "bind error" << std::endl;
      sqlite3_finalize(queryParentStmt);
      throw Error("bind error");
    }
  }
  sqlite3_finalize(queryParentStmt);
  return true;
}

bool
SqliteHandle::readNameSelector(const Interest& interest, vector<Name>& names) const
{
  if (interest.getName().empty()) {
    std::cerr << "The name of interest is empty" << std::endl;
    return false;
  }
  Name tmpName = interest.getName();
  //This queue is for internal node;
  queue<Name> internalNames;

  // Step 1. Check if the requested Data corresponds to a leaf (data is not NULL)
  sqlite3_stmt* queryStmt = 0;
  string sql = string("SELECT * FROM NDN_REPO WHERE name = ? AND data IS NOT NULL;");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc != SQLITE_OK)
    throw Error("prepare error");

  if (sqlite3_bind_blob(queryStmt, 1,
                        tmpName.wireEncode().wire(),
                        tmpName.wireEncode().size(), 0) == SQLITE_OK) {
    rc = sqlite3_step(queryStmt);
    if (rc == SQLITE_ROW) {
      Data elementData;
      elementData.wireDecode(Block(sqlite3_column_blob(queryStmt, 1),
                                   sqlite3_column_bytes(queryStmt, 1)));
      if (interest.matchesData(elementData)) {
        names.push_back(elementData.getName());
      }

      int nChildren = sqlite3_column_int(queryStmt, 3);
      if (nChildren == 0) {
        sqlite3_finalize(queryStmt);
        return true;
      }
    }
    else if (rc == SQLITE_DONE) {
      // ignore
    }
    else {
      std::cerr << "read error rc:" << rc << std::endl;
      sqlite3_finalize(queryStmt);
      throw Error("read error");
    }
  }
  sqlite3_finalize(queryStmt);

  // Step 2. Recursively find all data packets that match the Interest
  internalNames.push(tmpName);
  sqlite3_stmt* queryParentStmt = 0;
  string psql = string("SELECT * FROM NDN_REPO WHERE parentName = ?;");
  rc = sqlite3_prepare_v2(m_db, psql.c_str(), -1, &queryParentStmt, 0);
  if (rc != SQLITE_OK)
    throw Error("prepare error");

  while (!internalNames.empty()) {
    tmpName = internalNames.front();
    internalNames.pop();
    if (sqlite3_bind_blob(queryParentStmt, 1,
                          tmpName.wireEncode().wire(),
                          tmpName.wireEncode().size(), 0) == SQLITE_OK) {
      while (true) {
        rc = sqlite3_step(queryParentStmt);
        if (rc == SQLITE_ROW) {
          if (sqlite3_column_type(queryParentStmt, 1) != SQLITE_NULL) {
            Data elementData;
            elementData.wireDecode(Block(sqlite3_column_blob(queryParentStmt, 1),
                                         sqlite3_column_bytes(queryParentStmt, 1)));
            if (interest.matchesData(elementData)) {
              names.push_back(elementData.getName());
            }
          }

          Name elementName;
          elementName.wireDecode(Block(sqlite3_column_blob(queryParentStmt, 0),
                                       sqlite3_column_bytes(queryParentStmt, 0)));

          int nChildren = sqlite3_column_int(queryParentStmt, 3);
          if (nChildren > 0) {
            internalNames.push(elementName);
          }
        }
        else if (rc == SQLITE_DONE) {
          break;
        }
        else {
          std::cerr << "read error rc:" << rc << std::endl;
          sqlite3_finalize(queryParentStmt);
          throw Error("read error");
        }
      }
      sqlite3_reset(queryParentStmt);
    }
    else {
      std::cerr << "bind error" << std::endl;
      sqlite3_finalize(queryParentStmt);
      throw Error("bind error");
    }
  }
  sqlite3_finalize(queryParentStmt);
  return true;
}

bool
SqliteHandle::filterNameChild(const Name& name, int childSelector,
                              const vector<Name>& names, Name& resultName)
{
  BOOST_ASSERT(!names.empty());

  if (childSelector < 0) {
    resultName = *names.begin();
  }
  else if (childSelector == 0) {
    if (!names.empty()) {
      resultName = *std::min_element(names.begin(), names.end());
    }
    else {
      return false;
    }
  }
  else if (childSelector == 1) {
    if (!names.empty()) {
      resultName = *std::max_element(names.begin(), names.end());
    }
    else {
      return false;
    }
  }
  else {
    std::cerr << "Unknown ChildSelector specified" << std::endl;
    return false;
  }
  return true;
}

bool
SqliteHandle::readNameAny(const Name& name, const Selectors& selectors, vector<Name>& names)
{
  if (selectors.empty()) {
    if (hasName(name)) {
      names.push_back(name);
    }
    return true;
  }
  else {
    Interest interest(name);
    interest.setSelectors(selectors);
    readNameSelector(interest, names);
    if (names.empty())
      return false;
    if (selectors.getChildSelector() >= 0) {
      Name resultName;
      if (!filterNameChild(name, selectors.getChildSelector(), names, resultName))
        return false;
      names.clear();
      names.push_back(resultName);
      return true;
    }
    else {
      return true;
    }
  }
}

bool
SqliteHandle::readData(const Name& name, Data& data)
{
  sqlite3_stmt* queryStmt = 0;
  string sql = string("SELECT * FROM NDN_REPO WHERE name = ? AND data IS NOT NULL;");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_blob(queryStmt, 1,
                          name.wireEncode().wire(),
                          name.wireEncode().size(), 0) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        data.wireDecode(Block(sqlite3_column_blob(queryStmt, 1),
                              sqlite3_column_bytes(queryStmt, 1)));
        sqlite3_finalize(queryStmt);
        return true;
      }
      else if (rc == SQLITE_DONE) {
        return false;
      }
      else {
        std::cerr << "Database query failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        throw Error("Database query failure");
      }
    }
    sqlite3_finalize(queryStmt);
  }
  return true;
}


//This is the exact name query in database.
bool
SqliteHandle::hasName(const Name& name)
{
  sqlite3_stmt* queryStmt = 0;
  string sql = string("select * from NDN_REPO where name = ?;");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_blob(queryStmt, 1,
                          name.wireEncode().wire(),
                          name.wireEncode().size(), 0) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        sqlite3_finalize(queryStmt);
        return true;
      }
      else if (rc == SQLITE_DONE) {
        sqlite3_finalize(queryStmt);
        return false;
      }
      else {
        std::cerr << "Database query failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        return false;
      }
    }
    sqlite3_finalize(queryStmt);
  }
  return true;
}

//This is the exact parent name query in database.
bool
SqliteHandle::hasParentName(const Name& parentName) const
{
  sqlite3_stmt* queryStmt = 0;
  string sql = string("SELECT * FROM NDN_REPO WHERE parentName = ?;");
  int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &queryStmt, 0);
  if (rc == SQLITE_OK) {
    if (sqlite3_bind_blob(queryStmt, 1,
                          parentName.wireEncode().wire(),
                          parentName.wireEncode().size(), 0) == SQLITE_OK) {
      rc = sqlite3_step(queryStmt);
      if (rc == SQLITE_ROW) {
        sqlite3_finalize(queryStmt);
        return true;
      }
      else if (rc == SQLITE_DONE) {
        sqlite3_finalize(queryStmt);
        return false;
      }
      else {
        std::cerr << "Database query failure rc:" << rc << std::endl;
        sqlite3_finalize(queryStmt);
        return false;
      }
    }
    sqlite3_finalize(queryStmt);
  }
  return true;
}

size_t
SqliteHandle::size()
{
  sqlite3_stmt* queryStmt = 0;
  string sql("SELECT count(*) FROM NDN_REPO WHERE data IS NOT NULL");
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

  size_t nDatas = static_cast<size_t>(sqlite3_column_int64(queryStmt, 0));
  return nDatas;
}

} //namespace repo
