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
#include <vector>
#include <boost/mpl/vector.hpp>

namespace repo {
namespace tests {


class DatasetBase
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

  typedef std::list<ndn::shared_ptr<ndn::Data> > DataContainer;
  DataContainer data;

  typedef std::list<std::pair<ndn::Interest, ndn::shared_ptr<ndn::Data> > > InterestContainer;
  InterestContainer interests;

  typedef std::list<std::pair<ndn::Interest, size_t > > RemovalsContainer;
  RemovalsContainer removals;

protected:
  ndn::shared_ptr<ndn::Data>
  createData(const ndn::Name& name)
  {
    if (map.count(name) > 0)
      return map[name];

    static ndn::KeyChain keyChain;
    static std::vector<uint8_t> content(1500, '-');

    ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>();
    data->setName(name);
    data->setContent(&content[0], content.size());
    keyChain.sign(*data);

    map.insert(std::make_pair(name, data));
    return data;
  }

  ndn::shared_ptr<ndn::Data>
  getData(const ndn::Name& name)
  {
    if (map.count(name) > 0)
      return map[name];
    else
      throw Error("Data with name " + name.toUri() + " is not found");
  }

private:
  std::map<Name, shared_ptr<Data> > map;
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
      ndn::shared_ptr<Data> data = createData(name);
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

    this->interests.push_back(std::make_pair(Interest("/a"),       getData("/a/b/c/d")));
    this->interests.push_back(std::make_pair(Interest("/a/b"),     getData("/a/b/c/d")));
    this->interests.push_back(std::make_pair(Interest("/a/b/c"),   getData("/a/b/c/d")));
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


class BasicChildSelectorDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "BasicChildSelectorDataset";
    return name;
  }

  BasicChildSelectorDataset()
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


class ExtendedChildSelectorDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "storage";
    return name;
  }

  ExtendedChildSelectorDataset()
  {
    this->data.push_back(createData("/a/b/1"));

    this->data.push_back(createData("/a/c/1"));
    this->interests.push_back(std::make_pair(Interest("/a")
                                             .setSelectors(Selectors()
                                                           .setChildSelector(1)),
                                              this->data.back()));

    this->data.push_back(createData("/a/c/2"));

    this->data.push_back(createData("/b"));
  }
};


class ComplexSelectorsDataset : public DatasetBase
{
public:
  static const std::string&
  getName()
  {
    static std::string name = "ComplexSelectorsDataset";
    return name;
  }

  std::map<std::string, shared_ptr<Data> > map;

  void
  addData(const std::string& name)
  {
  }

  ComplexSelectorsDataset()
  {
    // Dataset
    this->data.push_back(createData("/a/b/c"));
    this->data.push_back(createData("/a/b/d/1"));
    this->data.push_back(createData("/a/b/d/2"));
    this->data.push_back(createData("/a/b/d/3"));
    this->data.push_back(createData("/a/b/d/4/I"));
    this->data.push_back(createData("/a/b/d/4"));
    this->data.push_back(createData("/a/b/d"));
    this->data.push_back(createData("/a/b/e/1"));
    this->data.push_back(createData("/a/b/e"));

    // Basic selects
    this->interests.push_back(std::make_pair(Interest("/a/b/c"),     this->getData("/a/b/c")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d"),     this->getData("/a/b/d/1")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d/1"),   this->getData("/a/b/d/1")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d/2"),   this->getData("/a/b/d/2")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d/3"),   this->getData("/a/b/d/3")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d/4/I"), this->getData("/a/b/d/4/I")));
    this->interests.push_back(std::make_pair(Interest("/a/b/d/4"),   this->getData("/a/b/d/4/I")));
    this->interests.push_back(std::make_pair(Interest("/a/b/e"),     this->getData("/a/b/e/1")));
    this->interests.push_back(std::make_pair(Interest("/a/b/e/1"),   this->getData("/a/b/e/1")));

    // Complex selects
    this->interests.push_back(std::make_pair(Interest("/a/b")
                                             .setSelectors(Selectors()
                                                           .setMinSuffixComponents(2)
                                                           .setMaxSuffixComponents(2)),
                                             this->getData("/a/b/c")));

    this->interests.push_back(std::make_pair(Interest("/a/b/d")
                                             .setSelectors(Selectors()
                                                           .setMinSuffixComponents(-1)
                                                           .setChildSelector(0)),
                                             this->getData("/a/b/d/1")));

    this->interests.push_back(std::make_pair(Interest("/a/b/d")
                                             .setSelectors(Selectors()
                                                           .setMinSuffixComponents(2)
                                                           .setChildSelector(0)),
                                             this->getData("/a/b/d/1")));

    this->interests.push_back(std::make_pair(
      Interest("/a/b/d")
      .setSelectors(Selectors()
                    .setChildSelector(1)
                    .setMaxSuffixComponents(2)
                    .setMinSuffixComponents(2)
                    .setExclude(Exclude()
                                .excludeRange(ndn::name::Component("3"),
                                              ndn::name::Component("4")))),
      this->getData("/a/b/d/2")));


    this->interests.push_back(std::make_pair(Interest("/a/b/d")
                                               .setSelectors(Selectors().setMinSuffixComponents(3)),
                                             this->getData("/a/b/d/4/I")));

    // According to selector definition, RightMost for the next level and LeftMost for the next-next level
    this->interests.push_back(std::make_pair(Interest("/a/b/d")
                                             .setSelectors(Selectors()
                                                           .setMinSuffixComponents(2)
                                                           .setChildSelector(1)),
                                             this->getData("/a/b/d/4/I")));

    // because of the digest component, /a/b/d will be to the right of /a/b/d/4
    this->interests.push_back(std::make_pair(Interest("/a/b/d")
                                             .setSelectors(Selectors()
                                                           .setChildSelector(1)),
                                             this->getData("/a/b/d")));

    // Alex: this interest doesn't make sense, as all Data packets will have the same selector
    this->interests.push_back(std::make_pair(Interest("/a/b/e")
                                             .setSelectors(Selectors()
                                                           .setPublisherPublicKeyLocator(
                                                             this->data.back()
                                                               ->getSignature().getKeyLocator())),
                                             this->getData("/a/b/e/1")));

    // Removals
    this->removals.push_back(std::make_pair(Interest("/a/b/d/2"), 1));

    this->removals.push_back(std::make_pair(
      Interest("/a/b/d")
      .setSelectors(Selectors()
                    .setMaxSuffixComponents(2)
                    .setMinSuffixComponents(2)
                    .setExclude(Exclude()
                                .excludeOne(ndn::name::Component("3")))),
      2));
  }
};


typedef boost::mpl::vector< BasicDataset,
                            FetchByPrefixDataset,
                            BasicChildSelectorDataset,
                            ExtendedChildSelectorDataset,
                            SamePrefixDataset<10>,
                            SamePrefixDataset<100> > CommonDatasets;


} // namespace tests
} // namespace repo

#endif // REPO_TESTS_DATASET_FIXTURES_HPP
