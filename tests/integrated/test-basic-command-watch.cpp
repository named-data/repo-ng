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

#include "handles/watch-handle.hpp"
#include "storage/sqlite-storage.hpp"

#include "command-fixture.hpp"
#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <ndn-cxx/util/random.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

using ndn::time::milliseconds;
using ndn::time::seconds;
using ndn::EventId;

// All the test cases in this test suite should be run at once.
BOOST_AUTO_TEST_SUITE(TestBasicCommandWatchDelete)

const static uint8_t content[8] = {3, 1, 4, 1, 5, 9, 2, 6};

template<class Dataset>
class Fixture : public CommandFixture, public RepoStorageFixture, public Dataset
{
public:
  Fixture()
    : watchHandle(repoFace, *handle, dispatcher, scheduler, validator)
    , watchFace(repoFace.getIoService())
  {
    Name cmdPrefix("/repo/command");
    repoFace.registerPrefix(cmdPrefix, nullptr,
      [] (const Name& cmdPrefix, const std::string& reason) {
        BOOST_FAIL("Command prefix registration error: " << reason);
      });
  }

  void
  scheduleWatchEvent();

  void
  onWatchInterest(const Interest& interest);

  void
  onRegisterFailed(const std::string& reason);

  void
  delayedInterest();

  void
  onWatchData(const Interest& interest, const Data& data);

  void
  onWatchStopData(const Interest& interest, const Data& data);

  void
  onWatchTimeout(const Interest& interest);

  void
  sendWatchStartInterest(const Interest& interest);

  void
  sendWatchStopInterest(const Interest& interest);

  void
  checkWatchOk(const Interest& interest);

public:
  WatchHandle watchHandle;
  Face watchFace;
  std::map<Name, EventId> watchEvents;
};

template<class T> void
Fixture<T>::onWatchInterest(const Interest& interest)
{
  auto data = make_shared<Data>(Name(interest.getName())
                                .appendNumber(ndn::random::generateWord64() + 100));
  data->setContent(content, sizeof(content));
  data->setFreshnessPeriod(0_ms);
  keyChain.sign(*data);
  watchFace.put(*data);

  // schedule an event 50ms later to check whether watch is Ok
  scheduler.scheduleEvent(10000_ms,
                          bind(&Fixture<T>::checkWatchOk, this,
                               Interest(data->getName())));
}


template<class T> void
Fixture<T>::onRegisterFailed(const std::string& reason)
{
  BOOST_ERROR("ERROR: Failed to register prefix in local hub's daemon" + reason);
}

template<class T> void
Fixture<T>::delayedInterest()
{
  BOOST_ERROR("Fetching interest does not come. It may be satisfied in CS or something is wrong");
}

template<class T> void
Fixture<T>::onWatchData(const Interest& interest, const Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());

  int statusCode = response.getCode();
  BOOST_CHECK_EQUAL(statusCode, 100);
}

template<class T> void
Fixture<T>::onWatchStopData(const Interest& interest, const Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());

  int statusCode = response.getCode();
  BOOST_CHECK_EQUAL(statusCode, 101);
}

template<class T> void
Fixture<T>::onWatchTimeout(const Interest& interest)
{
  BOOST_ERROR("Watch command timeout");
}

template<class T> void
Fixture<T>::sendWatchStartInterest(const Interest& watchInterest)
{
  watchFace.expressInterest(watchInterest,
                            bind(&Fixture<T>::onWatchData, this, _1, _2),
                            bind(&Fixture<T>::onWatchTimeout, this, _1), // Nack
                            bind(&Fixture<T>::onWatchTimeout, this, _1));
}

template<class T> void
Fixture<T>::sendWatchStopInterest(const Interest& watchInterest)
{
  watchFace.expressInterest(watchInterest,
                            bind(&Fixture<T>::onWatchStopData, this, _1, _2),
                            bind(&Fixture<T>::onWatchTimeout, this, _1), // Nack
                            bind(&Fixture<T>::onWatchTimeout, this, _1));
}

template<class T> void
Fixture<T>::checkWatchOk(const Interest& interest)
{
  BOOST_TEST_MESSAGE(interest);
  shared_ptr<Data> data = handle->readData(interest);
  if (data) {
    int rc = memcmp(data->getContent().value(), content, sizeof(content));
    BOOST_CHECK_EQUAL(rc, 0);
  }
  else {
    std::cerr<<"Check Watch Failed"<<std::endl;
  }
}

template<class T> void
Fixture<T>::scheduleWatchEvent()
{
  Name watchCommandName("/repo/command/watch/start");
  RepoCommandParameter watchParameter;
  watchParameter.setName(Name("/a/b"));
  watchParameter.setMaxInterestNum(10);
  watchParameter.setInterestLifetime(50000_ms);
  watchParameter.setWatchTimeout(1000000000_ms);
  watchCommandName.append(watchParameter.wireEncode());
  Interest watchInterest(watchCommandName);
  keyChain.sign(watchInterest);
  //schedule a job to express watchInterest
  scheduler.scheduleEvent(1000_ms,
                          bind(&Fixture<T>::sendWatchStartInterest, this, watchInterest));

  Name watchStopName("/repo/command/watch/stop");
  RepoCommandParameter watchStopParameter;
  watchStopName.append(watchStopParameter.wireEncode());
  Interest watchStopInterest(watchStopName);
  keyChain.sign(watchStopInterest);

  //The delayEvent will be canceled in onWatchInterest
  watchFace.setInterestFilter(watchParameter.getName(),
                              bind(&Fixture<T>::onWatchInterest, this, _2),
                              ndn::RegisterPrefixSuccessCallback(),
                              bind(&Fixture<T>::onRegisterFailed, this, _2));
}

typedef boost::mpl::vector<BasicDataset> Dataset;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(WatchDelete, T, Dataset, Fixture<T>)
{
  // schedule events
  this->scheduler.scheduleEvent(1_s,
                                bind(&Fixture<T>::scheduleWatchEvent, this));

  this->repoFace.processEvents(500_s);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
