/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022, Regents of the University of California.
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

#include "storage/repo-storage.hpp"
#include "../dataset-fixtures.hpp"
#include "../repo-storage-fixture.hpp"

#include <boost/test/unit_test.hpp>

namespace repo::tests {

BOOST_AUTO_TEST_SUITE(RepoStorage)

template<class Dataset>
class Fixture : public Dataset, public RepoStorageFixture
{
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(Bulk, T, CommonDatasets, Fixture<T>)
{
  // Insert data into repo
  for (auto i = this->data.begin(); i != this->data.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
  }

  // check size directly with the storage (repo doesn't have interface yet)
  BOOST_CHECK_EQUAL(this->store->size(), static_cast<int64_t>(this->data.size()));

  // Read
  for (auto i = this->interests.begin(); i != this->interests.end(); ++i) {
    auto dataTest = this->handle->readData(i->first);
    BOOST_CHECK_EQUAL(*dataTest, *i->second);
  }

  // Remove items
  for (auto i = this->removals.begin(); i != this->removals.end(); ++i) {
    size_t nRemoved = 0;
    nRemoved = this->handle->deleteData(i->first);
    BOOST_CHECK_EQUAL(nRemoved, i->second);
  }
}

BOOST_FIXTURE_TEST_CASE(NotifyAboutExistingData, Fixture<BasicDataset>)
{
  // Insert data into repo
  for (const auto& d : this->data) {
    handle->insertData(*d);
  }

  std::vector<Name> names;
  handle->afterDataInsertion.connect([&] (const Name& name) {
    names.push_back(name);
  });
  handle->notifyAboutExistingData();

  BOOST_CHECK_EQUAL(names.size(), this->data.size());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace repo::tests
