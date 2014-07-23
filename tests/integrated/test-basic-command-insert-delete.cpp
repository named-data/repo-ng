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

#include "handles/write-handle.hpp"
#include "handles/delete-handle.hpp"
#include "storage/sqlite-storage.hpp"
#include "storage/repo-storage.hpp"
#include "common.hpp"

#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/preprocessor/comparison/not_equal.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

namespace repo {
namespace tests {

using ndn::time::milliseconds;
using ndn::time::seconds;
using ndn::EventId;
namespace random=ndn::random;

//All the test cases in this test suite should be run at once.
BOOST_AUTO_TEST_SUITE(TestBasicCommandInsertDelete)

const static uint8_t content[8] = {3, 1, 4, 1, 5, 9, 2, 6};

template<class Dataset>
class Fixture : public RepoStorageFixture, public Dataset
{
public:
  Fixture()
    : scheduler(repoFace.getIoService())
    , validator(repoFace)
    , writeHandle(repoFace, *handle, keyChain, scheduler, validator)
    , deleteHandle(repoFace, *handle, keyChain, scheduler, validator)
    , insertFace(repoFace.getIoService())
    , deleteFace(repoFace.getIoService())
  {
    writeHandle.listen(Name("/repo/command"));
    deleteHandle.listen(Name("/repo/command"));
  }

  ~Fixture()
  {
    repoFace.getIoService().stop();
  }

  void
  generateDefaultCertificateFile();

  void
  scheduleInsertEvent();

  void
  scheduleDeleteEvent();

  void
  onInsertInterest(const Interest& interest);

  void
  onRegisterFailed(const std::string& reason);

  void
  delayedInterest();

  void
  stopFaceProcess();


  void
  onInsertData(const Interest& interest, Data& data);

  void
  onDeleteData(const Interest& interest, Data& data);

  void
  onInsertTimeout(const Interest& interest);

  void
  onDeleteTimeout(const Interest& interest);

  void
  sendInsertInterest(const Interest& interest);

  void
  sendDeleteInterest(const Interest& deleteInterest);

  void
  checkInsertOk(const Interest& interest);

  void
  checkDeleteOk(const Interest& interest);

public:
  Face repoFace;
  Scheduler scheduler;
  ValidatorConfig validator;
  KeyChain keyChain;
  WriteHandle writeHandle;
  DeleteHandle deleteHandle;
  Face insertFace;
  Face deleteFace;
  std::map<Name, EventId> insertEvents;
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
Fixture<T>::onInsertInterest(const Interest& interest)
{
  Data data(Name(interest.getName()));
  data.setContent(content, sizeof(content));
  data.setFreshnessPeriod(milliseconds(0));
  keyChain.signByIdentity(data, keyChain.getDefaultIdentity());
  insertFace.put(data);
  std::map<Name, EventId>::iterator event = insertEvents.find(interest.getName());
  if (event != insertEvents.end()) {
    scheduler.cancelEvent(event->second);
    insertEvents.erase(event);
  }
  // schedule an event 50ms later to check whether insert is Ok
  scheduler.scheduleEvent(milliseconds(500),
                          bind(&Fixture<T>::checkInsertOk, this, interest));

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
Fixture<T>::onInsertData(const Interest& interest, Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());
  int statusCode = response.getStatusCode();
  BOOST_CHECK_EQUAL(statusCode, 100);
  //  std::cout<<"statuse code of insert name = "<<response.getName()<<std::endl;
}

template<class T> void
Fixture<T>::onDeleteData(const Interest& interest, Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());
  int statusCode = response.getStatusCode();
  BOOST_CHECK_EQUAL(statusCode, 200);

  //schedlute an event to check whether delete is Ok.
  scheduler.scheduleEvent(milliseconds(100),
                          bind(&Fixture<T>::checkDeleteOk, this, interest));
}

template<class T> void
Fixture<T>::onInsertTimeout(const Interest& interest)
{
  BOOST_ERROR("Inserert command timeout");
}

template<class T> void
Fixture<T>::onDeleteTimeout(const Interest& interest)
{
  BOOST_ERROR("delete command timeout");
}

template<class T> void
Fixture<T>::sendInsertInterest(const Interest& insertInterest)
{
  insertFace.expressInterest(insertInterest,
                             bind(&Fixture<T>::onInsertData, this, _1, _2),
                             bind(&Fixture<T>::onInsertTimeout, this, _1));
}

template<class T> void
Fixture<T>::sendDeleteInterest(const Interest& deleteInterest)
{
  deleteFace.expressInterest(deleteInterest,
                             bind(&Fixture<T>::onDeleteData, this, _1, _2),
                             bind(&Fixture<T>::onDeleteTimeout, this, _1));
}

template<class T> void
Fixture<T>::checkInsertOk(const Interest& interest)
{
  BOOST_TEST_MESSAGE(interest);
  shared_ptr<Data> data = handle->readData(interest);
  if (data) {
    int rc = memcmp(data->getContent().value(), content, sizeof(content));
    BOOST_CHECK_EQUAL(rc, 0);
  }
  else {
    std::cerr<<"Check Insert Failed"<<std::endl;
  }
}

template<class T> void
Fixture<T>::checkDeleteOk(const Interest& interest)
{
  shared_ptr<Data> data = handle->readData(interest);
  BOOST_CHECK_EQUAL(data, shared_ptr<Data>());
}

template<class T> void
Fixture<T>::scheduleInsertEvent()
{
  int timeCount = 1;
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    Name insertCommandName("/repo/command/insert");
    RepoCommandParameter insertParameter;
    insertParameter.setName(Name((*i)->getName())
                              .appendNumber(random::generateWord64()));

    insertCommandName.append(insertParameter.wireEncode());
    Interest insertInterest(insertCommandName);
    keyChain.signByIdentity(insertInterest, keyChain.getDefaultIdentity());
    //schedule a job to express insertInterest every 50ms
    scheduler.scheduleEvent(milliseconds(timeCount * 50 + 1000),
                            bind(&Fixture<T>::sendInsertInterest, this, insertInterest));
    //schedule what to do when interest timeout

    EventId delayEventId = scheduler.scheduleEvent(milliseconds(5000 + timeCount * 50),
                                                   bind(&Fixture<T>::delayedInterest, this));
    insertEvents[insertParameter.getName()] = delayEventId;

    //The delayEvent will be canceled in onInsertInterest
    insertFace.setInterestFilter(insertParameter.getName(),
                                 bind(&Fixture<T>::onInsertInterest, this, _2),
                                 ndn::RegisterPrefixSuccessCallback(),
                                 bind(&Fixture<T>::onRegisterFailed, this, _2));
    timeCount++;
  }
}

template<class T> void
Fixture<T>::scheduleDeleteEvent()
{
  int timeCount = 1;
  for (typename T::DataContainer::iterator i = this->data.begin();
       i != this->data.end(); ++i) {
    Name deleteCommandName("/repo/command/delete");
    RepoCommandParameter deleteParameter;
    static boost::random::mt19937_64 gen;
    static boost::random::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFLL);
    deleteParameter.setProcessId(dist(gen));
    deleteParameter.setName((*i)->getName());
    deleteCommandName.append(deleteParameter.wireEncode());
    Interest deleteInterest(deleteCommandName);
    keyChain.signByIdentity(deleteInterest, keyChain.getDefaultIdentity());
    scheduler.scheduleEvent(milliseconds(4000 + timeCount * 50),
                            bind(&Fixture<T>::sendDeleteInterest, this, deleteInterest));
    timeCount++;
  }
}

typedef boost::mpl::vector< BasicDataset,
                            FetchByPrefixDataset,
                            BasicChildSelectorDataset,
                            ExtendedChildSelectorDataset,
                            SamePrefixDataset<10> > Datasets;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertDelete, T, Datasets, Fixture<T>)
{
  this->generateDefaultCertificateFile();
  this->validator.load("tests/integrated/insert-delete-validator-config.conf");

  // schedule events
  this->scheduler.scheduleEvent(seconds(0),
                                bind(&Fixture<T>::scheduleInsertEvent, this));
  this->scheduler.scheduleEvent(seconds(10),
                                bind(&Fixture<T>::scheduleDeleteEvent, this));

  // schedule an event to terminate IO
  this->scheduler.scheduleEvent(seconds(30),
                                bind(&Fixture<T>::stopFaceProcess, this));
  this->repoFace.getIoService().run();
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace tests
} //namespace repo
