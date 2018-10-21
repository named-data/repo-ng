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

#include "handles/read-handle.hpp"
#include "storage/sqlite-storage.hpp"
#include "storage/repo-storage.hpp"

#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/asio/io_service.hpp>
#include <boost/test/unit_test.hpp>

#include <ndn-cxx/util/time.hpp>

namespace repo {
namespace tests {

// All the test cases in this test suite should be run at once.
BOOST_AUTO_TEST_SUITE(TestBasicInerestRead)

const static uint8_t content[8] = {3, 1, 4, 1, 5, 9, 2, 6};

template<class Dataset>
class BasicInterestReadFixture : public RepoStorageFixture, public Dataset
{
public:
  BasicInterestReadFixture()
    : scheduler(repoFace.getIoService())
    , readHandle(repoFace, *handle, 0)
    , readFace(repoFace.getIoService())
  {
  }

  ~BasicInterestReadFixture()
  {
    repoFace.getIoService().stop();
  }

  void
  startListen()
  {
    readHandle.listen("/");
  }

  void
  scheduleReadEvent()
  {
    int timeCount = 1;
    for (typename Dataset::DataContainer::iterator i = this->data.begin();
         i != this->data.end(); ++i) {
      //First insert a data into database;
      (*i)->setContent(content, sizeof(content));
      (*i)->setFreshnessPeriod(36000_ms);
      keyChain.sign(**i);
      bool rc = handle->insertData(**i);

      BOOST_CHECK_EQUAL(rc, true);
      Interest readInterest((*i)->getName());
      readInterest.setMustBeFresh(true);
      scheduler.scheduleEvent(ndn::time::milliseconds(timeCount * 50),
                              bind(&BasicInterestReadFixture<Dataset>::sendInterest, this,
                                        readInterest));
      timeCount++;
    }
  }

  void
  onReadData(const ndn::Interest& interest, const ndn::Data& data)
  {
    int rc = memcmp(data.getContent().value(), content, sizeof(content));
    BOOST_CHECK_EQUAL(rc, 0);
  }

  void
  onReadTimeout(const ndn::Interest& interest)
  {
    BOOST_ERROR("Insert not successfull or Read data does not successfull");
  }

  void
  onReadNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
  {
    BOOST_ERROR("Read nacked");
  }

  void
  sendInterest(const ndn::Interest& interest)
  {
    readFace.expressInterest(interest,
                             bind(&BasicInterestReadFixture::onReadData, this, _1, _2),
                             bind(&BasicInterestReadFixture::onReadNack, this, _1, _2),
                             bind(&BasicInterestReadFixture::onReadTimeout, this, _1));
  }

public:
  ndn::Face repoFace;
  ndn::KeyChain keyChain;
  ndn::Scheduler scheduler;
  ReadHandle readHandle;
  ndn::Face readFace;
};

typedef boost::mpl::vector<BasicDataset,
                            FetchByPrefixDataset,
                            BasicChildSelectorDataset,
                            ExtendedChildSelectorDataset,
                            SamePrefixDataset<10>> Datasets;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(Read, T, Datasets, BasicInterestReadFixture<T>)
{
  this->startListen();
  this->scheduler.scheduleEvent(1_s,
                                bind(&BasicInterestReadFixture<T>::scheduleReadEvent, this));

  this->repoFace.processEvents(20_s);

}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
