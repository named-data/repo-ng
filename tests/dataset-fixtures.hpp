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

#ifndef REPO_TESTS_DATASET_FIXTURES_HPP
#define REPO_TESTS_DATASET_FIXTURES_HPP

#include <ndn-cxx/security/key-chain.hpp>

namespace repo {
namespace tests {

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


class DatasetBase
{
public:
  typedef std::list<ndn::shared_ptr<ndn::Data> > DataContainer;
  DataContainer data;

  typedef std::list<std::pair<ndn::Interest, ndn::shared_ptr<ndn::Data> > > InterestContainer;
  InterestContainer interests;
};


template<size_t N>
class BaseSmoketestFixture : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "BaseSmoketest";
    return name;
  }

  BaseSmoketestFixture()
  {
    ndn::Name baseName("/x/y/z/test/1");
    for (size_t i = 0; i < N; i++) {
      ndn::Name name(baseName);
      name.appendSegment(i);
      ndn::shared_ptr<Data> data = createData(name);
      this->data.push_back(data);

      this->interests.push_back(std::make_pair(Interest(name), data));
    }
  }
};


class BaseTestFixture : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "BaseTest";
    return name;
  }

  BaseTestFixture()
  {
    this->data.push_back(createData("/a"));
    this->interests.push_back(std::make_pair(Interest("/a"), this->data.back()));

    this->data.push_back(createData("/a/b"));
    this->interests.push_back(std::make_pair(Interest("/a/b"), this->data.back()));

    this->data.push_back(createData("/a/b/c"));
    this->interests.push_back(std::make_pair(Interest("/a/b/c"), this->data.back()));

    this->data.push_back(createData("/a/b/c/d"));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d"), this->data.back()));
  }
};


class FetchByPrefixTestFixture : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "FetchByPrefix";
    return name;
  }

  FetchByPrefixTestFixture()
  {
    this->data.push_back(createData("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z"));
    this->interests.push_back(std::make_pair(Interest("/a"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t"),
                                             this->data.back()));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u"),
                                             this->data.back()));
    this->interests.push_back(
       std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v"),
                      this->data.back()));
    this->interests.push_back(
       std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w"),
                      this->data.back()));
    this->interests.push_back(
      std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x"),
                     this->data.back()));
    this->interests.push_back(
      std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y"),
                     this->data.back()));
    this->interests.push_back(
      std::make_pair(Interest("/a/b/c/d/e/f/g/h/i/j/k/l/m/n/o/p/q/r/s/t/u/v/w/x/y/z"),
                     this->data.back()));
  }
};


class SelectorTestFixture : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "SelectorTest";
    return name;
  }

  SelectorTestFixture()
  {
    this->data.push_back(createData("/a/1"));
    this->data.push_back(createData("/b/1"));
    this->interests.push_back(std::make_pair(Interest()
                                         .setName("/b")
                                         .setSelectors(Selectors()
                                                         .setChildSelector(0)),
                                       this->data.back()));

    this->data.push_back(createData("/c/1"));
    this->data.push_back(createData("/b/99"));
    this->interests.push_back(std::make_pair(Interest()
                                         .setName("/b")
                                         .setSelectors(Selectors()
                                                         .setChildSelector(1)),
                                       this->data.back()));
    this->data.push_back(createData("/b/5"));
    this->data.push_back(createData("/b/55"));
  }
};


typedef boost::mpl::vector< BaseTestFixture,
                            FetchByPrefixTestFixture,
                            SelectorTestFixture,
                            BaseSmoketestFixture<1>,
                            BaseSmoketestFixture<100> > DatasetFixtures;

} // namespace tests
} // namespace repo

#endif // REPO_TESTS_DATASET_FIXTURES_HPP
