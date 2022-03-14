/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022,  Regents of the University of California.
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

#include <boost/test/unit_test.hpp>
#include <ndn-cxx/util/dummy-client-face.hpp>

#define CHECK_INTERESTS(NAME, COMPONENT, EXPECTED)                   \
  do {                                                               \
    bool didMatch = false;                                           \
    for (const auto& interest : face.sentInterests) {                \
      didMatch = didMatch || containsNameComponent(NAME, COMPONENT); \
    }                                                                \
    BOOST_CHECK_EQUAL(didMatch, EXPECTED);                           \
  } while (false)

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(TestReadHandle)

class Fixture : public RepoStorageFixture
{
public:
  Fixture()
    : face({true, true})
    , scheduler(face.getIoService())
    , subsetLength(1)
    , dataPrefix("/ndn/test/prefix")
    , identity("/ndn/test/identity")
    , readHandle(face, *handle, subsetLength)
    , numPrefixRegistrations(0)
    , numPrefixUnregistrations(0)
  {
    readHandle.connectAutoListen();
  }

  static bool
  containsNameComponent(const Name& name, const ndn::name::Component& component)
  {
    for (const auto& c : name) {
      if (c == component)
        return true;
    }
    return false;
  }

public:
  ndn::util::DummyClientFace face;
  ndn::KeyChain keyChain;
  ndn::Scheduler scheduler;

  size_t subsetLength;
  ndn::Name dataPrefix;
  ndn::Name identity;
  ReadHandle readHandle;

  size_t numPrefixRegistrations;
  size_t numPrefixUnregistrations;
};

BOOST_FIXTURE_TEST_CASE(DataPrefixes, Fixture)
{
  const std::vector<uint8_t> content(100, 'x');
  auto data1 = std::make_shared<Data>(Name{dataPrefix}.appendNumber(1));
  auto data2 = std::make_shared<Data>(Name{dataPrefix}.appendNumber(2));

  data1->setContent(content);
  data2->setContent(content);

  keyChain.createIdentity(identity);
  keyChain.sign(*data1, ndn::security::SigningInfo(ndn::security::SigningInfo::SIGNER_TYPE_ID,
                                                  identity));
  keyChain.sign(*data2, ndn::security::SigningInfo(ndn::security::SigningInfo::SIGNER_TYPE_ID,
                                                  identity));

  face.sentInterests.clear();
  handle->insertData(*data1);
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"register"}, true);

  face.sentInterests.clear();
  handle->deleteData(data1->getFullName());
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"unregister"}, true);

  face.sentInterests.clear();
  handle->insertData(*data1);
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"register"}, true);

  face.sentInterests.clear();
  handle->insertData(*data2);
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"register"}, false);

  face.sentInterests.clear();
  handle->deleteData(data1->getFullName());
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"unregister"}, false);

  face.sentInterests.clear();
  handle->deleteData(data2->getFullName());
  face.processEvents(-1_ms);
  CHECK_INTERESTS(interest.getName(), name::Component{"unregister"}, true);
}

BOOST_AUTO_TEST_SUITE_END() // TestReadHandle

} // namespace tests
} // namespace repo
