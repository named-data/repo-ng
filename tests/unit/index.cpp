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

#include "storage/index.hpp"

#include "../sqlite-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/test/unit_test.hpp>
#include <iostream>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(TestIndex)

template<class Dataset>
class Fixture : public Dataset
{
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(IndexGeneralTest, T, DatasetFixtures_Storage, Fixture<T>)
{
  Index index(65535);
  for (typename T::IdContainer::iterator i = this->insert.begin();
       i != this->insert.end(); ++i)
  {
    BOOST_CHECK_EQUAL(index.insert(*i->second, i->first), true);
  }
  BOOST_CHECK_EQUAL(index.size(), 7);

  typename T::IdContainer::iterator id = this->ids.begin();
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i)
  {
    vector<std::pair<int, ndn::Name> > id_names;
    BOOST_CHECK_EQUAL(index.find(i->first.getName()).first, id->first);
    BOOST_CHECK_EQUAL(index.hasData(*i->second), true);
    ++id;
  }

  for (typename T::InterestIdContainer::iterator i = this->interestsSelectors.begin();
       i != this->interestsSelectors.end(); ++i)
  {
    BOOST_CHECK_EQUAL(index.find(i->first).first, i->second);
    ndn::Name name = index.find(i->first).second;
    BOOST_CHECK_EQUAL(index.erase(name), true);
  }
  BOOST_CHECK_EQUAL(index.size(), 2);
}


BOOST_FIXTURE_TEST_CASE_TEMPLATE(IndexTestSelector, T, DatasetFixtures_Selector, Fixture<T>)
{
  Index index(65535);
  for (typename T::IdContainer::iterator i = this->insert.begin();
       i != this->insert.end(); ++i)
    BOOST_CHECK_EQUAL(index.insert(*i->second, i->first), true);
  for (typename T::InterestIdContainer::iterator i = this->interestsSelectors.begin();
       i != this->interestsSelectors.end(); ++i)
  {
    BOOST_CHECK_EQUAL(index.find(i->first).first, i->second);
  }
}

class FindFixture
{
protected:
  FindFixture()
    : m_index(std::numeric_limits<size_t>::max())
  {
  }

  void
  insert(int id, const Name& name)
  {
    shared_ptr<Data> data = make_shared<Data>(name);
    data->setContent(reinterpret_cast<const uint8_t*>(&id), sizeof(id));
    m_keyChain.signWithSha256(*data);
    data->wireEncode();
    m_index.insert(*data, id);
  }

  Interest&
  startInterest(const Name& name)
  {
    m_interest = make_shared<Interest>(name);
    return *m_interest;
  }

  int
  find()
  {
    std::pair<int,Name> found = m_index.find(*m_interest);
    return found.first;
  }

protected:
  Index m_index;
  KeyChain m_keyChain;
  shared_ptr<Interest> m_interest;
};

BOOST_FIXTURE_TEST_SUITE(Find, FindFixture)

BOOST_AUTO_TEST_CASE(EmptyDataName)
{
  insert(1, "ndn:/");
  startInterest("ndn:/");
  BOOST_CHECK_EQUAL(find(), 1);
}

BOOST_AUTO_TEST_CASE(EmptyInterestName)
{
  insert(1, "ndn:/A");
  startInterest("ndn:/");
  BOOST_CHECK_EQUAL(find(), 1);
}

BOOST_AUTO_TEST_CASE(Leftmost)
{
  insert(1, "ndn:/A");
  insert(2, "ndn:/B/p/1");
  insert(3, "ndn:/B/p/2");
  insert(4, "ndn:/B/q/1");
  insert(5, "ndn:/B/q/2");
  insert(6, "ndn:/C");

  startInterest("ndn:/B");
  BOOST_CHECK_EQUAL(find(), 2);
}

BOOST_AUTO_TEST_CASE(Rightmost)
{
  insert(1, "ndn:/A");
  insert(2, "ndn:/B/p/1");
  insert(3, "ndn:/B/p/2");
  insert(4, "ndn:/B/q/1");
  insert(5, "ndn:/B/q/2");
  insert(6, "ndn:/C");

  startInterest("ndn:/B")
    .setChildSelector(1);
  BOOST_CHECK_EQUAL(find(), 4);
}

BOOST_AUTO_TEST_CASE(Leftmost_ExactName1)
{
  insert(1, "ndn:/");
  insert(2, "ndn:/A/B");
  insert(3, "ndn:/A/C");
  insert(4, "ndn:/A");
  insert(5, "ndn:/D");

  // Intuitively you would think Data 4 should be between Data 1 and 2,
  // but Data 4 has full Name ndn:/A/<32-octet hash>.
  startInterest("ndn:/A");
  BOOST_CHECK_EQUAL(find(), 2);
}

BOOST_AUTO_TEST_CASE(Leftmost_ExactName33)
{
  insert(1, "ndn:/");
  insert(2, "ndn:/A");
  insert(3, "ndn:/A/BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"); // 33 'B's
  insert(4, "ndn:/A/CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"); // 33 'C's
  insert(5, "ndn:/D");

  // Data 2 is returned, because <32-octet hash> is less than Data 3.
  startInterest("ndn:/A");
  BOOST_CHECK_EQUAL(find(), 2);
}

BOOST_AUTO_TEST_CASE(MinSuffixComponents)
{
  insert(1, "ndn:/A/1/2/3/4");
  insert(2, "ndn:/B/1/2/3");
  insert(3, "ndn:/C/1/2");
  insert(4, "ndn:/D/1");
  insert(5, "ndn:/E");
  insert(6, "ndn:/");

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(0);
  BOOST_CHECK_EQUAL(find(), 6);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(1);
  BOOST_CHECK_EQUAL(find(), 6);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(2);
  BOOST_CHECK_EQUAL(find(), 5);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(3);
  BOOST_CHECK_EQUAL(find(), 4);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(4);
  BOOST_CHECK_EQUAL(find(), 3);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(5);
  BOOST_CHECK_EQUAL(find(), 2);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(6);
  BOOST_CHECK_EQUAL(find(), 1);

  startInterest("ndn:/")
    .setChildSelector(1)
    .setMinSuffixComponents(7);
  BOOST_CHECK_EQUAL(find(), 0);
}

BOOST_AUTO_TEST_CASE(MaxSuffixComponents)
{
  insert(1, "ndn:/");
  insert(2, "ndn:/A");
  insert(3, "ndn:/A/B");
  insert(4, "ndn:/A/B/C");
  insert(5, "ndn:/A/B/C/D");
  insert(6, "ndn:/A/B/C/D/E");
  // Order is 6,5,4,3,2,1, because <32-octet hash> is greater than a 1-octet component.

  startInterest("ndn:/")
    .setMaxSuffixComponents(0);
  BOOST_CHECK_EQUAL(find(), 0);

  startInterest("ndn:/")
    .setMaxSuffixComponents(1);
  BOOST_CHECK_EQUAL(find(), 1);

  startInterest("ndn:/")
    .setMaxSuffixComponents(2);
  BOOST_CHECK_EQUAL(find(), 2);

  startInterest("ndn:/")
    .setMaxSuffixComponents(3);
  BOOST_CHECK_EQUAL(find(), 3);

  startInterest("ndn:/")
    .setMaxSuffixComponents(4);
  BOOST_CHECK_EQUAL(find(), 4);

  startInterest("ndn:/")
    .setMaxSuffixComponents(5);
  BOOST_CHECK_EQUAL(find(), 5);

  startInterest("ndn:/")
    .setMaxSuffixComponents(6);
  BOOST_CHECK_EQUAL(find(), 6);
}

BOOST_AUTO_TEST_CASE(DigestOrder)
{
  insert(1, "ndn:/A");
  insert(2, "ndn:/A");
  // We don't know which comes first, but there must be some order

  startInterest("ndn:/A")
    .setChildSelector(0);
  uint32_t leftmost = find();

  startInterest("ndn:/A")
    .setChildSelector(1);
  uint32_t rightmost = find();

  BOOST_CHECK_NE(leftmost, rightmost);
}

BOOST_AUTO_TEST_CASE(DigestExclude)
{
  insert(1, "ndn:/A/B");
  insert(2, "ndn:/A");
  insert(3, "ndn:/A/CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"); // 33 'C's

  startInterest("ndn:/A")
    .setExclude(Exclude().excludeBefore(Name::Component(reinterpret_cast<const uint8_t*>(
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"
        "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"), 31))); // 31 0xFF's
  BOOST_CHECK_EQUAL(find(), 2);

  startInterest("ndn:/A")
    .setChildSelector(1)
    .setExclude(Exclude().excludeAfter(Name::Component(reinterpret_cast<const uint8_t*>(
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00"), 33))); // 33 0x00's
  BOOST_CHECK_EQUAL(find(), 2);
}

BOOST_AUTO_TEST_CASE(ExactName32)
{
  insert(1, "ndn:/A/BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"); // 32 'B's
  insert(2, "ndn:/A/CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"); // 32 'C's

  startInterest("ndn:/A/BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
  BOOST_CHECK_EQUAL(find(), 1);
}

BOOST_AUTO_TEST_CASE(MinSuffixComponents32)
{
  insert(1, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A/1/2/3/4"); // 32 'x's
  insert(2, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/B/1/2/3");
  insert(3, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/C/1/2");
  insert(4, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/D/1");
  insert(5, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/E");
  insert(6, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(0);
  BOOST_CHECK_EQUAL(find(), 6);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(1);
  BOOST_CHECK_EQUAL(find(), 6);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(2);
  BOOST_CHECK_EQUAL(find(), 5);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(3);
  BOOST_CHECK_EQUAL(find(), 4);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(4);
  BOOST_CHECK_EQUAL(find(), 3);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(5);
  BOOST_CHECK_EQUAL(find(), 2);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(6);
  BOOST_CHECK_EQUAL(find(), 1);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setChildSelector(1)
    .setMinSuffixComponents(7);
  BOOST_CHECK_EQUAL(find(), 0);
}

BOOST_AUTO_TEST_CASE(MaxSuffixComponents32)
{
  insert(1, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/"); // 32 'x's
  insert(2, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A");
  insert(3, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A/B");
  insert(4, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A/B/C");
  insert(5, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A/B/C/D");
  insert(6, "ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx/A/B/C/D/E");
  // Order is 6,5,4,3,2,1, because <32-octet hash> is greater than a 1-octet component.

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(0);
  BOOST_CHECK_EQUAL(find(), 0);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(1);
  BOOST_CHECK_EQUAL(find(), 1);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(2);
  BOOST_CHECK_EQUAL(find(), 2);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(3);
  BOOST_CHECK_EQUAL(find(), 3);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(4);
  BOOST_CHECK_EQUAL(find(), 4);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(5);
  BOOST_CHECK_EQUAL(find(), 5);

  startInterest("ndn:/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx")
    .setMaxSuffixComponents(6);
  BOOST_CHECK_EQUAL(find(), 6);
}

BOOST_AUTO_TEST_SUITE_END() // Find

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
