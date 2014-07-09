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

#include "storage/repo-storage.hpp"
#include "storage/sqlite-storage.hpp"
#include "../dataset-fixtures.hpp"
#include "../repo-storage-fixture.hpp"

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string.h>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(RepoStorage)
template<class Dataset>
class Fixture : public Dataset, public RepoStorageFixture
{
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NdnNameSkipList, T, DatasetFixtures_Storage, Fixture<T>)
{
  //Insert
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) 
    {
      BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
    }
  
  //Read
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) 
  {
      shared_ptr<ndn::Data> dataTest = this->handle->readData(i->first);
      BOOST_CHECK_EQUAL(*this->handle->readData(i->first), *i->second);
     // int rc = memcmp(dataTest->getContent().value(),
      //                i->second->getContent().value(), sizeof(i->second->getContent().value()));
      //BOOST_CHECK_EQUAL(rc, 0);
      BOOST_CHECK_EQUAL(this->handle->deleteData(i->first.getName()), 1);
    }
 
  //Insert
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) 
    {
      BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
    }

  //Erase
  for (typename T::InterestIdContainer::iterator i = this->interestDeleteCount.begin();
       i != this->interestDeleteCount.end(); ++i)
    {
      BOOST_CHECK_EQUAL(this->handle->deleteData(i->first), i->second);
    }
}

BOOST_FIXTURE_TEST_CASE_TEMPLATE(Index, T, DatasetFixtures_Storage, Fixture<T>)
{

  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i)
    {
      BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
    }
  ndn::Interest interest("/a");
  ndn::Interest interest1("/a/b/d/1");

  BOOST_CHECK_EQUAL(this->handle->deleteData(interest), 7);


  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i)
    {
      BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
    }
  BOOST_CHECK_EQUAL(this->handle->deleteData(interest.getName()), 7);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
