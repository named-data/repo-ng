/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "../storage/sqlite/sqlite-handle.hpp"

#include <ndn-cpp-dev/security/key-chain.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp>

namespace repo {

class TestDbCreateDestroy
{
public:
  TestDbCreateDestroy()
  {
    boost::filesystem::path testPath("unittestdb");
    boost::filesystem::file_status testPathStatus = boost::filesystem::status(testPath);
    if (!boost::filesystem::is_directory(testPathStatus)) {
      if (!boost::filesystem::create_directory(boost::filesystem::path(testPath))) {
        BOOST_FAIL("Cannot create unittestdb folder");
      }
    }
    handle = new SqliteHandle("unittestdb");
  }

  ~TestDbCreateDestroy()
  {
    delete handle;
    boost::filesystem::remove_all(boost::filesystem::path("unittestdb"));
  }

public:
  SqliteHandle* handle;
};

BOOST_FIXTURE_TEST_SUITE(TestSqlite, TestDbCreateDestroy)

static inline ndn::shared_ptr<ndn::Data>
createData(const ndn::Name& name)
{
  static ndn::KeyChain keyChain;
  static std::vector<uint8_t> content(1500, '-');

  ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>();
  data->setName(name);
  data->setContent(&content[0], content.size());
  keyChain.sign(*data);

  return data;
}

class FixtureBase : public TestDbCreateDestroy
{
public:
  typedef std::list<ndn::shared_ptr<ndn::Data> > DataContainer;
  DataContainer datas;

  typedef std::list<std::pair<ndn::Interest, ndn::shared_ptr<ndn::Data> > > InterestContainer;
  InterestContainer interests;
};

template<size_t N>
class BaseSmoketestFixture : public FixtureBase
{
public:
  BaseSmoketestFixture()
  {
    ndn::Name baseName("/x/y/z/test/1");
    for (size_t i = 0; i < N; i++) {
      ndn::Name name(baseName);
      name.appendSegment(i);
      ndn::shared_ptr<Data> data = createData(name);
      this->datas.push_back(data);

      this->interests.push_back(std::make_pair(Interest(name), data));
    }
  }
};

class BaseTestFixture : public FixtureBase
{
public:
  BaseTestFixture()
  {
    datas.push_back(createData("/a"));
    interests.push_back(std::make_pair(Interest("/a"), datas.back()));

    datas.push_back(createData("/a/b"));
    interests.push_back(std::make_pair(Interest("/a/b"), datas.back()));

    datas.push_back(createData("/a/b/c"));
    interests.push_back(std::make_pair(Interest("/a/b/c"), datas.back()));

    datas.push_back(createData("/a/b/c/d"));
    interests.push_back(std::make_pair(Interest("/a/b/c/d"), datas.back()));
  }
};

class SelectorTestFixture : public FixtureBase
{
public:
  SelectorTestFixture()
  {
    datas.push_back(createData("/a/1"));
    datas.push_back(createData("/b/1"));
    interests.push_back(std::make_pair(Interest()
                                         .setName("/b")
                                         .setSelectors(Selectors()
                                                         .setChildSelector(0)),
                                       datas.back()));

    datas.push_back(createData("/c/1"));
    datas.push_back(createData("/b/99"));
    interests.push_back(std::make_pair(Interest()
                                         .setName("/b")
                                         .setSelectors(Selectors()
                                                         .setChildSelector(1)),
                                       datas.back()));
    datas.push_back(createData("/b/5"));
    datas.push_back(createData("/b/55"));
  }
};

typedef boost::mpl::vector< BaseTestFixture,
                            SelectorTestFixture,
                            BaseSmoketestFixture<1>,
                            BaseSmoketestFixture<100> > Fixtures;

BOOST_FIXTURE_TEST_CASE_TEMPLATE(InsertReadDelete, T, Fixtures, T)
{
  // Insert
  for (typename T::DataContainer::iterator i = this->datas.begin();
       i != this->datas.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->insertData(**i), true);
  }

  // Read (all items should exist)
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) {
    ndn::Data retrievedData;
    BOOST_REQUIRE_EQUAL(this->handle->readData(i->first, retrievedData), true);
    BOOST_CHECK_EQUAL(retrievedData, *i->second);
  }

  // Delete
  for (typename T::DataContainer::iterator i = this->datas.begin();
       i != this->datas.end(); ++i) {
    BOOST_CHECK_EQUAL(this->handle->deleteData((*i)->getName()), true);
  }

  // Read (none of the items should exist)
  for (typename T::InterestContainer::iterator i = this->interests.begin();
       i != this->interests.end(); ++i) {
    ndn::Data retrievedData;
    BOOST_REQUIRE_EQUAL(this->handle->readData(i->first, retrievedData), false);
  }
}

BOOST_AUTO_TEST_SUITE_END()

}// namespace repo
