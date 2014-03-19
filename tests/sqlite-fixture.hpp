/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_TESTS_SQLITE_FIXTURE_HPP
#define REPO_TESTS_SQLITE_FIXTURE_HPP

#include "../storage/sqlite/sqlite-handle.hpp"

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

class SqliteFixture
{
public:
  SqliteFixture()
  {
    handle = new SqliteHandle("unittestdb");
  }

  ~SqliteFixture()
  {
    delete handle;
    boost::filesystem::remove_all(boost::filesystem::path("unittestdb"));
  }

public:
  SqliteHandle* handle;
};

} // namespace tests
} // namespace repo

#endif // REPO_TESTS_SQLITE_FIXTURE_HPP
