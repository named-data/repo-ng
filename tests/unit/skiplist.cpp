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

#include "storage/skiplist.hpp"

#include "../sqlite-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/concept_check.hpp>
#include <iostream>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(SkipList)

template<class Dataset>
class Fixture : public Dataset
{
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(NdnNameSkipList, T, DatasetFixtures_Index, Fixture<T>)
{
  repo::SkipList<ndn::Name, typename T::DataSetNameCompare> skipList;
  //Insert
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    skipList.insert((*i)->getName());
  }

  //find and erase
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    typename repo::SkipList<ndn::Name, typename T::DataSetNameCompare>::iterator findIterator =
      skipList.lower_bound((*i)->getName());
    skipList.erase(findIterator);
  }

  //@todo test lower_bound
}

BOOST_AUTO_TEST_CASE(IntGtSkipList)
{
  typedef repo::SkipList<int, std::greater<int> > IntGtSkipList;
  IntGtSkipList sl;
  BOOST_CONCEPT_ASSERT((boost::BidirectionalIterator<IntGtSkipList::iterator>));

  // initial state
  BOOST_CHECK_EQUAL(sl.size(), 0);
  BOOST_CHECK(sl.begin() == sl.end());
  BOOST_CHECK(sl.lower_bound(10) == sl.end());

  // initial contents
  sl.insert(10);
  sl.insert(20);
  sl.insert(30);
  BOOST_CHECK_EQUAL(sl.size(), 3);
  // contents: [30,20,10]

  // iterators
  IntGtSkipList::iterator it1 = sl.begin();
  IntGtSkipList::iterator it2 = sl.end();
  --it2;
  BOOST_CHECK_EQUAL(*it1, 30);
  BOOST_CHECK_EQUAL(*it2, 10);
  ++it1;
  BOOST_CHECK_EQUAL(*it1, 20);
  IntGtSkipList::iterator it3 = it1;
  ++it1;
  BOOST_CHECK(it2 == it1);
  BOOST_CHECK_EQUAL(*it3, 20);

  // lower_bound
  IntGtSkipList::iterator found = sl.lower_bound(35);
  BOOST_CHECK(found == sl.begin());
  BOOST_CHECK_EQUAL(*found, 30);
  found = sl.lower_bound(30);
  BOOST_CHECK_EQUAL(*found, 30);
  found = sl.lower_bound(25);
  BOOST_CHECK_EQUAL(*found, 20);
  found = sl.lower_bound(20);
  BOOST_CHECK_EQUAL(*found, 20);
  found = sl.lower_bound(15);
  BOOST_CHECK_EQUAL(*found, 10);
  found = sl.lower_bound(10);
  BOOST_CHECK_EQUAL(*found, 10);
  found = sl.lower_bound(5);
  BOOST_CHECK(found == sl.end());

  // insert duplicate
  std::pair<IntGtSkipList::iterator, bool> insertRes = sl.insert(10);
  BOOST_CHECK_EQUAL(insertRes.second, false);
  BOOST_CHECK_EQUAL(*(insertRes.first), 10);
  BOOST_CHECK_EQUAL(sl.size(), 3);
  insertRes = sl.insert(20);
  BOOST_CHECK_EQUAL(insertRes.second, false);
  BOOST_CHECK_EQUAL(*(insertRes.first), 20);
  BOOST_CHECK_EQUAL(sl.size(), 3);
  insertRes = sl.insert(30);
  BOOST_CHECK_EQUAL(insertRes.second, false);
  BOOST_CHECK_EQUAL(*(insertRes.first), 30);
  BOOST_CHECK_EQUAL(sl.size(), 3);

  // insert non-duplicate
  insertRes = sl.insert(5);
  BOOST_CHECK_EQUAL(insertRes.second, true);
  BOOST_CHECK_EQUAL(*(insertRes.first), 5);
  BOOST_CHECK_EQUAL(sl.size(), 4);
  insertRes = sl.insert(35);
  BOOST_CHECK_EQUAL(insertRes.second, true);
  BOOST_CHECK_EQUAL(*(insertRes.first), 35);
  BOOST_CHECK_EQUAL(sl.size(), 5);
  // contents: [35,30,20,10,5]

  // erase
  it1 = sl.erase(sl.begin());
  // contents: [30,20,10,5]
  BOOST_CHECK_EQUAL(*it1, 30);
  BOOST_CHECK_EQUAL(sl.size(), 4);
  it2 = sl.end();
  --it2;
  it1 = sl.erase(it2);
  // contents: [30,20,10]
  BOOST_CHECK(it1 == sl.end());
  BOOST_CHECK_EQUAL(sl.size(), 3);
  it2 = sl.lower_bound(20);
  it1 = sl.erase(it2);
  // contents: [30,10]
  BOOST_CHECK_EQUAL(*it1, 10);
  BOOST_CHECK_EQUAL(sl.size(), 2);
  it3 = it1;
  --it1;
  BOOST_CHECK(it1 == sl.begin());
  BOOST_CHECK_EQUAL(*it1, 30);
  ++it3;
  BOOST_CHECK(it3 == sl.end());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
