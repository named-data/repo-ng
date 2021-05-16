/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2021, Regents of the University of California.
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

#include "handles/delete-handle.hpp"
#include "handles/write-handle.hpp"

#include "storage/repo-storage.hpp"
#include "storage/sqlite-storage.hpp"

#include "command-fixture.hpp"
#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <ndn-cxx/security/interest-signer.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/util/time.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

using ndn::time::milliseconds;

// All the test cases in this test suite should be run at once.
BOOST_AUTO_TEST_SUITE(TestBasicCommandInsertDelete)

const static uint8_t content[8] = {3, 1, 4, 1, 5, 9, 2, 6};

template<class Dataset>
class Fixture : public CommandFixture, public RepoStorageFixture, public Dataset
{
public:
  Fixture()
    : writeHandle(repoFace, *handle, dispatcher, scheduler, validator)
    , deleteHandle(repoFace, *handle, dispatcher, scheduler, validator)
    , insertFace(repoFace.getIoService())
    , deleteFace(repoFace.getIoService())
    , signer(keyChain)
  {
    Name cmdPrefix("/repo/command");
    repoFace.registerPrefix(cmdPrefix, nullptr,
      [] (const Name& cmdPrefix, const std::string& reason) {
        BOOST_FAIL("Command prefix registration error: " << reason);
      });
  }

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
  onInsertData(const Interest& interest, const Data& data);

  void
  onDeleteData(const Interest& interest, const Data& data);

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
  WriteHandle writeHandle;
  DeleteHandle deleteHandle;
  Face insertFace;
  Face deleteFace;
  std::map<Name, ndn::scheduler::EventId> insertEvents;
  std::map<Name, Name> deleteNamePairs;
  ndn::security::InterestSigner signer;
};

template<class T> void
Fixture<T>::onInsertInterest(const Interest& interest)
{
  Data data(Name(interest.getName()));
  data.setContent(content, sizeof(content));
  data.setFreshnessPeriod(0_ms);
  keyChain.sign(data);
  insertFace.put(data);
  auto eventIt = insertEvents.find(interest.getName());
  if (eventIt != insertEvents.end()) {
    eventIt->second.cancel();
    insertEvents.erase(eventIt);
  }
  // schedule an event 50ms later to check whether insert is Ok
  scheduler.schedule(500_ms, std::bind(&Fixture<T>::checkInsertOk, this, interest));
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
Fixture<T>::onInsertData(const Interest& interest, const Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());
  int statusCode = response.getCode();
  BOOST_CHECK_EQUAL(statusCode, 100);
}

template<class T> void
Fixture<T>::onDeleteData(const Interest& interest, const Data& data)
{
  RepoCommandResponse response;
  response.wireDecode(data.getContent().blockFromValue());
  int statusCode = response.getCode();
  BOOST_CHECK_EQUAL(statusCode, 200);

  //schedlute an event to check whether delete is Ok.
  scheduler.schedule(100_ms, std::bind(&Fixture<T>::checkDeleteOk, this, interest));
}

template<class T> void
Fixture<T>::onInsertTimeout(const Interest& interest)
{
  BOOST_ERROR("Insert command timeout");
}

template<class T> void
Fixture<T>::onDeleteTimeout(const Interest& interest)
{
  BOOST_ERROR("Delete command timeout");
}

template<class T> void
Fixture<T>::sendInsertInterest(const Interest& insertInterest)
{
  insertFace.expressInterest(insertInterest,
                             std::bind(&Fixture<T>::onInsertData, this, _1, _2),
                             std::bind(&Fixture<T>::onInsertTimeout, this, _1), // Nack
                             std::bind(&Fixture<T>::onInsertTimeout, this, _1));
}

template<class T> void
Fixture<T>::sendDeleteInterest(const Interest& deleteInterest)
{
  deleteFace.expressInterest(deleteInterest,
                             std::bind(&Fixture<T>::onDeleteData, this, _1, _2),
                             std::bind(&Fixture<T>::onDeleteTimeout, this, _1), // Nack
                             std::bind(&Fixture<T>::onDeleteTimeout, this, _1));
}

template<class T> void
Fixture<T>::checkInsertOk(const Interest& interest)
{
  BOOST_TEST_MESSAGE(interest);
  std::shared_ptr<Data> data = handle->readData(interest);
  if (data) {
    int rc = memcmp(data->getContent().value(), content, sizeof(content));
    BOOST_CHECK_EQUAL(rc, 0);
  }
  else {
    BOOST_ERROR("Check Insert Failed");
  }
}

template<class T> void
Fixture<T>::checkDeleteOk(const Interest& interest)
{
  std::map<Name, Name>::iterator name = deleteNamePairs.find(interest.getName());
  BOOST_CHECK_MESSAGE(name != deleteNamePairs.end(), "Delete name not found: " << interest.getName());
  Interest dataInterest(name->second);
  std::shared_ptr<Data> data = handle->readData(dataInterest);
  BOOST_CHECK(!data);
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
                            .appendNumber(ndn::random::generateWord64()));
    insertCommandName.append(insertParameter.wireEncode());
    Interest insertInterest = signer.makeCommandInterest(insertCommandName);
    // schedule a job to express insertInterest every 50ms
    scheduler.schedule(milliseconds(timeCount * 50 + 1000),
                       std::bind(&Fixture<T>::sendInsertInterest, this, insertInterest));
    // schedule what to do when interest timeout
    auto delayEventId = scheduler.schedule(milliseconds(5000 + timeCount * 50),
                                           std::bind(&Fixture<T>::delayedInterest, this));
    insertEvents[insertParameter.getName()] = delayEventId;
    // The delayEvent will be canceled in onInsertInterest
    insertFace.setInterestFilter(insertParameter.getName(),
                                 std::bind(&Fixture<T>::onInsertInterest, this, _2),
                                 ndn::RegisterPrefixSuccessCallback(),
                                 std::bind(&Fixture<T>::onRegisterFailed, this, _2));
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
    deleteParameter.setProcessId(ndn::random::generateWord64());
    deleteParameter.setName((*i)->getName());
    deleteCommandName.append(deleteParameter.wireEncode());
    Interest deleteInterest = signer.makeCommandInterest(deleteCommandName);
    deleteNamePairs[deleteInterest.getName()] = (*i)->getName();
    scheduler.schedule(milliseconds(4000 + timeCount * 50),
                       std::bind(&Fixture<T>::sendDeleteInterest, this, deleteInterest));
    timeCount++;
  }
}

using Datasets = boost::mpl::vector<BasicDataset,
                                    FetchByPrefixDataset,
                                    SamePrefixDataset<10>>;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertDelete, T, Datasets, Fixture<T>)
{
  // schedule events
  this->scheduler.schedule(0_s, std::bind(&Fixture<T>::scheduleInsertEvent, this));
  this->scheduler.schedule(10_s, std::bind(&Fixture<T>::scheduleDeleteEvent, this));

  this->repoFace.processEvents(30_s);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
