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

/**
 * @file This file demonstrates how to generate data to be stored in a repo using
 *       the repo watch protocol and repo insertion protocol.
 *
 * The details of the protocols can be found here:
 *  <https://redmine.named-data.net/projects/repo-ng/wiki/Watched_Prefix_Insertion_Protocol>
 *  <https://redmine.named-data.net/projects/repo-ng/wiki/Basic_Repo_Insertion_Protocol>
 *
 * This file is used for debugging purpose. There are two modes for users to assign
 * names for the data.
 * 1)read the data name from a specific file
 * 2)input a prefix and a random version number will be added automatically
 * Users need to run nfd and repo-ng and set up specific repo protocols mentioned
 * above before running this program.
 * The description of command parameter can be found in the function usage().
 */

#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

#include <fstream>
#include <iostream>
#include <random>
#include <string>

namespace repo {

using ndn::time::milliseconds;

const milliseconds DEFAULT_TIME_INTERVAL(2000);

enum Mode {
  AUTO,
  READFILE
};

class Publisher
{
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

public:
  Publisher()
    : mode(AUTO)
    , dataPrefix("/example/data")
    , timeInterval(DEFAULT_TIME_INTERVAL)
    , duration(0)
    , m_scheduler(m_face.getIoService())
    , m_randomDist(200, 1000)
  {
  }

  void
  run();

  void
  autoGenerate();

  void
  generateFromFile();

  static std::shared_ptr<ndn::Data>
  createData(const ndn::Name& name);

public:
  std::ifstream insertStream;
  Mode mode;
  ndn::Name dataPrefix;
  milliseconds timeInterval;
  milliseconds duration;

private:
  ndn::Face m_face;
  ndn::Scheduler m_scheduler;
  std::uniform_int_distribution<> m_randomDist;
};

void
Publisher::run()
{
  if (mode == AUTO) {
    m_scheduler.schedule(timeInterval, [this] { autoGenerate(); });
  }
  else {
    m_scheduler.schedule(timeInterval, [this] { generateFromFile(); });
  }
  m_face.processEvents(duration);
}

void
Publisher::autoGenerate()
{
  ndn::Name name = dataPrefix;
  name.appendNumber(m_randomDist(ndn::random::getRandomNumberEngine()));
  auto data = createData(name);
  m_face.put(*data);

  m_scheduler.schedule(timeInterval, [this] { autoGenerate(); });
}

void
Publisher::generateFromFile()
{
  if (insertStream.eof()) {
    m_face.getIoService().stop();
    return;
  }

  std::string name;
  getline(insertStream, name);
  auto data = createData(name);
  m_face.put(*data);

  m_scheduler.schedule(timeInterval, [this] { generateFromFile(); });
}

std::shared_ptr<ndn::Data>
Publisher::createData(const ndn::Name& name)
{
  static ndn::KeyChain keyChain;
  static const std::vector<uint8_t> content(1500, '-');

  auto data = std::make_shared<ndn::Data>(name);
  data->setContent(content);
  keyChain.sign(*data);
  return data;
}

static void
usage()
{
  std::cerr
      << " Publisher [-d dataPrefix] [-f filename] [-s duration time] [-t generate time interval] \n"
      << "  -d: specify the data prefix publisher generate\n"
      << "  -f: specify filename that publish would read from\n"
      << "  -s: specify the time duration of generate data\n"
      << "  -t: specify the time interval between two data generated\n"
      << std::endl;
  exit(1);
}

static int
main(int argc, char* argv[])
{
  Publisher generator;
  bool isAuto = false;
  bool isRead = false;
  int opt;
  while ((opt = getopt(argc, argv, "d:f:s:t:")) != -1) {
    switch (opt) {
    case 'd':
      {
        generator.dataPrefix = ndn::Name(std::string(optarg));
        generator.mode = AUTO;
        isAuto = true;
      }
      break;
    case 'f':
      {
        isRead = true;
        generator.mode = READFILE;
        std::string str = std::string(optarg);
        generator.insertStream.open(str.c_str());
         if (!generator.insertStream.is_open()) {
          std::cerr << "ERROR: cannot open " << std::string(optarg) << std::endl;
          return 1;
        }
      }
      break;
    case 's':
      try {
        generator.duration = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-s option should be an integer" << std::endl;
        return 1;
      }
      break;
    case 't':
      try {
        generator.timeInterval = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-t option should be an integer" << std::endl;
        return 1;
      }
      break;
    default:
      usage();
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 0)
    usage();

  if (isAuto && isRead)
    usage();

  generator.run();
  return 0;
}

} // namespace repo

int
main(int argc, char* argv[])
{
  try {
    return repo::main(argc, argv);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
}
