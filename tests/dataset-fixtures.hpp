/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
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

#ifndef REPO_TESTS_DATASET_FIXTURES_HPP
#define REPO_TESTS_DATASET_FIXTURES_HPP

#include "identity-management-fixture.hpp"
#include <vector>
#include <boost/mpl/vector.hpp>

namespace repo {
namespace tests {

class DatasetBase : public virtual IdentityManagementFixture
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  using DataContainer = std::list<std::shared_ptr<ndn::Data>>;
  DataContainer data;

  using InterestContainer = std::list<std::pair<ndn::Interest, std::shared_ptr<ndn::Data>>>;
  InterestContainer interests;

  using RemovalsContainer = std::list<std::pair<ndn::Interest, size_t>>;
  RemovalsContainer removals;

protected:
  std::shared_ptr<ndn::Data>
  createData(const ndn::Name& name)
  {
    if (map.count(name) > 0)
      return map[name];

    static std::vector<uint8_t> content(1500, '-');

    std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();
    data->setName(name);
    data->setContent(&content[0], content.size());
    m_keyChain.sign(*data);

    map.insert(std::make_pair(name, data));
    return data;
  }

  std::shared_ptr<ndn::Data>
  getData(const ndn::Name& name)
  {
    if (map.count(name) > 0)
      return map[name];
    else
      BOOST_THROW_EXCEPTION(Error("Data with name " + name.toUri() + " is not found"));
  }

private:
  std::map<Name, std::shared_ptr<Data>> map;
};


template<size_t N>
class SamePrefixDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "SamePrefixDataset";
    return name;
  }

  SamePrefixDataset()
  {
    ndn::Name baseName("/x/y/z/test/1");
    for (size_t i = 0; i < N; i++) {
      ndn::Name name(baseName);
      name.appendSegment(i);
      std::shared_ptr<Data> data = createData(name);
      this->data.push_back(data);

      this->interests.push_back(std::make_pair(Interest(name), data));
    }
  }
};


class BasicDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "BasicDataset";
    return name;
  }

  BasicDataset()
  {
    this->data.push_back(createData("/a"));
    this->data.push_back(createData("/a/b"));
    this->data.push_back(createData("/a/b/c"));
    this->data.push_back(createData("/a/b/c/d"));

    this->interests.push_back(std::make_pair(Interest("/a"),       getData("/a")));
    this->interests.push_back(std::make_pair(Interest("/a/b"),     getData("/a/b")));
    this->interests.push_back(std::make_pair(Interest("/a/b/c"),   getData("/a/b/c")));
    this->interests.push_back(std::make_pair(Interest("/a/b/c/d"), getData("/a/b/c/d")));
  }
};

//Fetch by prefix is useless due to the database is fetched by id
class FetchByPrefixDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "FetchByPrefixDataset";
    return name;
  }

  FetchByPrefixDataset()
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

using CommonDatasets = boost::mpl::vector<BasicDataset,
                                          FetchByPrefixDataset,
                                          SamePrefixDataset<10>,
                                          SamePrefixDataset<100>>;
} // namespace tests
} // namespace repo

#endif // REPO_TESTS_DATASET_FIXTURES_HPP
