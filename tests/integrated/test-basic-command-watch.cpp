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

#include "handles/watch-handle.hpp"
#include "storage/sqlite-storage.hpp"
#include "common.hpp"

#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/util/io.hpp>

#include <boost/test/unit_test.hpp>
#include <fstream>

namespace repo {
namespace tests {

using ndn::time::milliseconds;
using ndn::time::seconds;
using ndn::EventId;
namespace random=ndn::random;

//All the test cases in this test suite should be run at once.
BOOST_AUTO_TEST_SUITE(TestBasicCommandWatchDelete)

const static uint8_t content[8] = {3, 1, 4, 1, 5, 9, 2, 6};

template<class Dataset>
class Fixture : public RepoStorageFixture, public Dataset
{
public:
  Fixture()
    : scheduler(repoFace.getIoService())
    , validator(repoFace)
    , watchHandle(repoFace, *handle, keyChain, scheduler, validator)
    , watchFace(repoFace.getIoService())
  {
    watchHandle.listen(Name("/repo/command"));
  }

  ~Fixture()
  {
    repoFace.getIoService().stop();
  }

  void
  generateDefaultCertificateFile();

  void
  scheduleWatchEvent();

  void
  onWatchInterest(const Interest& interest);

  void
  onRegisterFailed(const std::string& reason);

  void
  delayedInterest();

  void
  stopFaceProcess();

  void
  onWatchData(const Interest& interest, Data& data);

  void
  onWatchStopData(const Interest& interest, Data& data);

  void
  onWatchTimeout(const Interest& interest);

  void
  sendWatchStartInterest(const Interest& interest);

  void
  sendWatchStopInterest(const Interest& interest);

  void
  checkWatchOk(const Interest& interest);

public:
  Face repoFace;
  Scheduler scheduler;
  ValidatorConfig validator;
  KeyChain keyChain;
  WatchHandle watchHandle;
  Face watchFace;
  std::map<Name, EventId> watchEvents;
};

template<class T>  void
Fixture<T>::generateDefaultCertificateFile()
{
  Name defaultIdentity = keyChain.getDefaultIdentity();
  Name defaultKeyname = keyChain.getDefaultKeyNameForIdentity(defaultIdentity);
  Name defaultCertficateName = keyChain.getDefaultCertificateNameForKey(defaultKeyname);
  shared_ptr<ndn::IdentityCertificate> defaultCertficate =
  keyChain.getCertificate(defaultCertficateName);
  //test-integrated should run in root directory of repo-ng.
  //certificate file should be removed after tests for security issue.
  std::fstream certificateFile("tests/integrated/insert-delete-test.cert",
                               std::ios::out | std::ios::binary | std::ios::trunc);
  ndn::io::save(*defaultCertficate, certificateFile);
  certificateFile.close();
}

template<class T> void
Fixture<T>::onWatchInterest(const Interest& interest)
{
  shared_ptr<Data> data = make_shared<Data>(Name(interest.getName()).appendNumber(random::generateWord64()+100));
  data->setContent(content, sizeof(content));
  data->setFreshnessPeriod(milliseconds(0));
  keyChain.signByIdentity(*data, keyChain.getDefaultIdentity());
  watchFace.put(*data);

  // schedule an event 50ms later to check whether watch is Ok
  scheduler.scheduleEvent(milliseconds(10000),
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
Fixture<T>::stopFaceProcess()
{
  repoFace.getIoService().stop();
}

template<class T> void
Fixture<T>::onWatchData(const Interest& interest, Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());

  int statusCode = response.getStatusCode();
  BOOST_CHECK_EQUAL(statusCode, 100);
}

template<class T> void
Fixture<T>::onWatchStopData(const Interest& interest, Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());

  int statusCode = response.getStatusCode();
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
                            bind(&Fixture<T>::onWatchTimeout, this, _1));
}

template<class T> void
Fixture<T>::sendWatchStopInterest(const Interest& watchInterest)
{
  watchFace.expressInterest(watchInterest,
                            bind(&Fixture<T>::onWatchStopData, this, _1, _2),
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
  watchParameter.setInterestLifetime(milliseconds(50000));
  watchParameter.setWatchTimeout(milliseconds(1000000000));
  watchCommandName.append(watchParameter.wireEncode());
  Interest watchInterest(watchCommandName);
  keyChain.signByIdentity(watchInterest, keyChain.getDefaultIdentity());
  //schedule a job to express watchInterest
  scheduler.scheduleEvent(milliseconds(1000),
                          bind(&Fixture<T>::sendWatchStartInterest, this, watchInterest));

  Name watchStopName("/repo/command/watch/stop");
  RepoCommandParameter watchStopParameter;
  watchStopName.append(watchStopParameter.wireEncode());
  Interest watchStopInterest(watchStopName);
  keyChain.signByIdentity(watchStopInterest, keyChain.getDefaultIdentity());

 // scheduler.scheduleEvent(milliseconds(10000),
  //                        bind(&Fixture<T>::sendWatchStopInterest, this, watchStopInterest));
  //The delayEvent will be canceled in onWatchInterest
  watchFace.setInterestFilter(watchParameter.getName(),
                              bind(&Fixture<T>::onWatchInterest, this, _2),
                              ndn::RegisterPrefixSuccessCallback(),
                              bind(&Fixture<T>::onRegisterFailed, this, _2));
}

typedef boost::mpl::vector< BasicDataset > Dataset;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(WatchDelete, T, Dataset, Fixture<T>)
{
  this->generateDefaultCertificateFile();
  this->validator.load("tests/integrated/insert-delete-validator-config.conf");

  // schedule events
  this->scheduler.scheduleEvent(seconds(0),
                                bind(&Fixture<T>::scheduleWatchEvent, this));

  // schedule an event to terminate IO
  this->scheduler.scheduleEvent(seconds(500),
                                bind(&Fixture<T>::stopFaceProcess, this));
  this->repoFace.getIoService().run();
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace tests
} //namespace repo
