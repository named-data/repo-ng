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

// Combine CommonDatasets with ComplexSelectorDataset
typedef boost::mpl::push_back<CommonDatasets,
                              ComplexSelectorsDataset>::type Datasets;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(Bulk, T, Datasets, Fixture<T>)
{
  // typedef ComplexSelectorsDataset T;
  BOOST_TEST_MESSAGE(T::getName());

  // Insert data into repo
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i)
    {
      BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
    }

  // check size directly with the storage (repo doesn't have interface yet)
  BOOST_CHECK_EQUAL(this->store->size(), this->data.size());

  // Read
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i)
  {
      shared_ptr<ndn::Data> dataTest = this->handle->readData(i->first);
      BOOST_CHECK_EQUAL(*this->handle->readData(i->first), *i->second);
    }

  // Remove items
  for (typename T::RemovalsContainer::iterator i = this->removals.begin();
       i != this->removals.end(); ++i)
    {
      size_t nRemoved = 0;
      BOOST_REQUIRE_NO_THROW(nRemoved = this->handle->deleteData(i->first));
      BOOST_CHECK_EQUAL(nRemoved, i->second);
    }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
