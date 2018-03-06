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

#include "storage/sqlite-storage.hpp"

#include "../sqlite-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <random>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(SqliteStorage)

template<class Dataset>
class Fixture : public SqliteFixture, public Dataset
{
public:
  std::map<Name, std::shared_ptr<Data>> nameToDataMap;
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertReadDelete, T, CommonDatasets, Fixture<T>)
{
  BOOST_TEST_CHECKPOINT(T::getName());

  std::vector<Name> names;

  // Insert
  for (auto i = this->data.begin();
       i != this->data.end(); ++i) {
    Name name = Name();
    this->handle->insert(**i);
    name = (*i)->getFullName();
    this->nameToDataMap.insert(std::make_pair(name, *i));
    names.push_back(name);
  }
  BOOST_CHECK_EQUAL(this->handle->size(), static_cast<int64_t>(this->data.size()));

  std::mt19937 rng{std::random_device{}()};
  std::shuffle(names.begin(), names.end(), rng);

  // Read (all items should exist)
  for (auto i = names.begin(); i != names.end(); ++i) {
    std::shared_ptr<Data> retrievedData = this->handle->read(*i);

    BOOST_REQUIRE(this->nameToDataMap.count(*i) > 0);
    BOOST_CHECK_EQUAL(*this->nameToDataMap[*i], *retrievedData);
  }
  BOOST_CHECK_EQUAL(this->handle->size(), static_cast<int64_t>(this->data.size()));

  // Delete
  for (auto i = names.begin(); i != names.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->erase(*i), true);
  }

  BOOST_CHECK_EQUAL(this->handle->size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
