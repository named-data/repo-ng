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
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertReadDelete, T, DatasetFixtures_Sqlite, Fixture<T>)
{
  BOOST_TEST_MESSAGE(T::getName());

  // Insert
  for (typename T::IdContainer::iterator i = this->ids.begin();
       i != this->ids.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->insert(*i->second), i->first);
  }

  BOOST_CHECK_EQUAL(this->handle->size(), this->data.size());

  // Read (all items should exist)
  for (typename T::IdContainer::iterator i = this->ids.begin();
       i != this->ids.end(); ++i) {
    shared_ptr<Data> retrievedData = this->handle->read(i->first);
    BOOST_CHECK_EQUAL(*retrievedData, *i->second);
  }

  // Delete
  for (typename T::IdContainer::iterator i = this->ids.begin();
       i != this->ids.end(); ++i) {
      //std::cout<<"remove name = "<<i->second->getName()<<std::endl;
    BOOST_CHECK_EQUAL(this->handle->erase(i->first), true);
  }

  /*
  // Read (none of the items should exist)
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) {
    ndn::Data retrievedData;
    BOOST_REQUIRE_EQUAL(this->handle->readData(i->first, retrievedData), false);
  }*/

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
