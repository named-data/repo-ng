/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "../storage/sqlite/sqlite-handle.hpp"

#include "sqlite-fixture.hpp"
#include "dataset-fixtures.hpp"

#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(SqliteHandle)

template<class Dataset>
class Fixture : public SqliteFixture, public Dataset
{
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertReadDelete, T, DatasetFixtures, Fixture<T>)
{
  BOOST_TEST_MESSAGE(T::getName());

  // Insert
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
  }

  BOOST_CHECK_EQUAL(this->handle->size(), this->data.size());

  // Read (all items should exist)
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) {
    ndn::Data retrievedData;
    BOOST_REQUIRE_EQUAL(this->handle->readData(i->first, retrievedData), true);
    BOOST_CHECK_EQUAL(retrievedData, *i->second);
  }

  // Delete
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->deleteData((*i)->getName()), true);
  }

  // Read (none of the items should exist)
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) {
    ndn::Data retrievedData;
    BOOST_REQUIRE_EQUAL(this->handle->readData(i->first, retrievedData), false);
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
