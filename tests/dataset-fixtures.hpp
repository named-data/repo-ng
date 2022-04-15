/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2022, Regents of the University of California.
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

namespace repo::tests {

class DatasetBase : public virtual IdentityManagementFixture
{
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  std::list<std::shared_ptr<Data>> data;
  std::list<std::pair<Interest, std::shared_ptr<Data>>> interests;
  std::list<std::pair<Interest, size_t>> removals;

protected:
  std::shared_ptr<Data>
  createData(const Name& name)
  {
    auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second;

    static const std::vector<uint8_t> content(1500, '-');

    auto data = std::make_shared<Data>(name);
    data->setContent(content);
    m_keyChain.sign(*data);

    m_map.emplace(name, data);
    return data;
  }

  std::shared_ptr<Data>
  getData(const Name& name) const
  {
    auto it = m_map.find(name);
    if (it != m_map.end())
      return it->second;

    NDN_THROW(Error("Data with name " + name.toUri() + " not found"));
  }

private:
  std::map<Name, std::shared_ptr<Data>> m_map;
};

template<size_t N>
class SamePrefixDataset : public DatasetBase
{
public:
  SamePrefixDataset()
  {
    const Name baseName("/x/y/z/test/1");
    for (size_t i = 0; i < N; i++) {
      Name name(baseName);
      name.appendSegment(i);
      auto data = createData(name);
      this->data.push_back(data);
      this->interests.emplace_back(Interest(name), data);
    }
  }
};

class BasicDataset : public DatasetBase
{
public:
  BasicDataset()
  {
    this->data.push_back(createData("/a"));
    this->data.push_back(createData("/a/b"));
    this->data.push_back(createData("/a/b/c"));
    this->data.push_back(createData("/a/b/c/d"));

    this->interests.emplace_back(Interest("/a"),       getData("/a"));
    this->interests.emplace_back(Interest("/a/b"),     getData("/a/b"));
    this->interests.emplace_back(Interest("/a/b/c"),   getData("/a/b/c"));
    this->interests.emplace_back(Interest("/a/b/c/d"), getData("/a/b/c/d"));
  }
};

//Fetch by prefix is useless due to the database is fetched by id
class FetchByPrefixDataset : public DatasetBase
{
public:
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
} // namespace repo::tests

#endif // REPO_TESTS_DATASET_FIXTURES_HPP
