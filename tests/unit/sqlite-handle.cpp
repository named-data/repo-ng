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

#include "storage/sqlite-storage.hpp"

#include "../sqlite-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(SqliteStorage)

template<class Dataset>
class Fixture : public SqliteFixture, public Dataset
{
public:
  std::map<int64_t, shared_ptr<Data> > idToDataMap;
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertReadDelete, T, CommonDatasets, Fixture<T>)
{
  BOOST_TEST_MESSAGE(T::getName());

  std::vector<int64_t> ids;

  // Insert
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i)
    {
      int64_t id = -1;
      BOOST_REQUIRE_NO_THROW(id = this->handle->insert(**i));

      this->idToDataMap.insert(std::make_pair(id, *i));
      ids.push_back(id);
    }
  BOOST_CHECK_EQUAL(this->handle->size(), this->data.size());

  std::random_shuffle(ids.begin(), ids.end());

  // Read (all items should exist)
  for (std::vector<int64_t>::iterator i = ids.begin(); i != ids.end(); ++i) {
    shared_ptr<Data> retrievedData = this->handle->read(*i);

    BOOST_REQUIRE(this->idToDataMap.count(*i) > 0);
    BOOST_CHECK_EQUAL(*this->idToDataMap[*i], *retrievedData);
  }
  BOOST_CHECK_EQUAL(this->handle->size(), this->data.size());

  // Delete
  for (std::vector<int64_t>::iterator i = ids.begin(); i != ids.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->erase(*i), true);
  }

  BOOST_CHECK_EQUAL(this->handle->size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
