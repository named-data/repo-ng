/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2024, Regents of the University of California.
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

#ifndef REPO_TESTS_REPO_STORAGE_FIXTURE_HPP
#define REPO_TESTS_REPO_STORAGE_FIXTURE_HPP

#include "storage/repo-storage.hpp"
#include "storage/sqlite-storage.hpp"

#include <filesystem>

namespace repo::tests {

class RepoStorageFixture
{
public:
  ~RepoStorageFixture()
  {
    std::error_code ec;
    std::filesystem::remove_all(std::filesystem::path("unittestdb"), ec);
  }

public:
  std::shared_ptr<Storage> store = std::make_shared<SqliteStorage>("unittestdb");
  std::shared_ptr<RepoStorage> handle = std::make_shared<RepoStorage>(*store);
};

} // namespace repo::tests

#endif // REPO_TESTS_REPO_STORAGE_FIXTURE_HPP
