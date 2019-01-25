/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019, Regents of the University of California.
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

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <boost/lexical_cast.hpp>

namespace repo {

using ndn::Name;
using ndn::Interest;
using ndn::Data;

class Consumer : boost::noncopyable
{
public:
  Consumer(const std::string& dataName, std::ostream& os,
           bool verbose, bool versioned, bool single,
           int interestLifetime, int timeout,
           bool mustBeFresh = false,
           bool canBePrefix = false)
    : m_dataName(dataName)
    , m_os(os)
    , m_verbose(verbose)
    , m_hasVersion(versioned)
    , m_isSingle(single)
    , m_isFinished(false)
    , m_isFirst(true)
    , m_interestLifetime(interestLifetime)
    , m_timeout(timeout)
    , m_nextSegment(0)
    , m_totalSize(0)
    , m_retryCount(0)
    , m_mustBeFresh(mustBeFresh)
    , m_canBePrefix(canBePrefix)
  {
  }

  void
  run();

private:
  void
  fetchData(const ndn::Name& name);

  void
  onVersionedData(const ndn::Interest& interest, const ndn::Data& data);

  void
  onUnversionedData(const ndn::Interest& interest, const ndn::Data& data);

  void
  onTimeout(const ndn::Interest& interest);

  void
  readData(const ndn::Data& data);

  void
  fetchNextData(const ndn::Name& name, const ndn::Data& data);

private:
  ndn::Face m_face;
  ndn::Name m_dataName;
  std::ostream& m_os;
  bool m_verbose;
  bool m_hasVersion;
  bool m_isSingle;
  bool m_isFinished;
  bool m_isFirst;
  ndn::time::milliseconds m_interestLifetime;
  ndn::time::milliseconds m_timeout;
  uint64_t m_nextSegment;
  int m_totalSize;
  int m_retryCount;
  bool m_mustBeFresh;
  bool m_canBePrefix;

  static constexpr int MAX_RETRY = 3;
};

void
Consumer::fetchData(const Name& name)
{
  Interest interest(name);
  interest.setInterestLifetime(m_interestLifetime);
  if (m_hasVersion) {
    interest.setMustBeFresh(m_mustBeFresh);
  }
  else {
    interest.setMustBeFresh(true);
  }

  interest.setCanBePrefix(m_canBePrefix);

  m_face.expressInterest(interest,
                         m_hasVersion ? std::bind(&Consumer::onVersionedData, this, _1, _2)
                                      : std::bind(&Consumer::onUnversionedData, this, _1, _2),
                         std::bind(&Consumer::onTimeout, this, _1), // Nack
                         std::bind(&Consumer::onTimeout, this, _1));
}

void
Consumer::run()
{
  // Send the first Interest
  Name name(m_dataName);

  m_nextSegment++;
  fetchData(name);

  // processEvents will block until the requested data received or timeout occurs
  m_face.processEvents(m_timeout);
}

void
Consumer::onVersionedData(const Interest& interest, const Data& data)
{
  const Name& name = data.getName();

  // the received data name may have segment number or not
  if (name.size() == m_dataName.size()) {
    if (!m_isSingle) {
      Name fetchName = name;
      fetchName.appendSegment(0);
      fetchData(fetchName);
    }
  }
  else if (name.size() == m_dataName.size() + 1) {
    if (!m_isSingle) {
      if (m_isFirst) {
        uint64_t segment = name[-1].toSegment();
        if (segment != 0) {
          fetchData(Name(m_dataName).appendSegment(0));
          m_isFirst = false;
          return;
        }
        m_isFirst = false;
      }
      fetchNextData(name, data);
    }
    else {
      std::cerr << "ERROR: Data is not stored in a single packet" << std::endl;
      return;
    }
  }
  else {
    std::cerr << "ERROR: Name size does not match" << std::endl;
    return;
  }
  readData(data);
}

void
Consumer::onUnversionedData(const Interest& interest, const Data& data)
{
  const Name& name = data.getName();
  if (name.size() == m_dataName.size() + 1) {
    if (!m_isSingle) {
      Name fetchName = name;
      fetchName.append(name[-1]).appendSegment(0);
      fetchData(fetchName);
    }
  }
  else if (name.size() == m_dataName.size() + 2) {
    if (!m_isSingle) {
       if (m_isFirst) {
        uint64_t segment = name[-1].toSegment();
        if (segment != 0) {
          fetchData(Name(m_dataName).append(name[-2]).appendSegment(0));
          m_isFirst = false;
          return;
        }
        m_isFirst = false;
      }
      fetchNextData(name, data);
    }
    else {
      std::cerr << "ERROR: Data is not stored in a single packet" << std::endl;
      return;
    }
  }
  else {
    std::cerr << "ERROR: Name size does not match" << std::endl;
    return;
  }
  readData(data);
}

void
Consumer::readData(const Data& data)
{
  const auto& content = data.getContent();
  m_os.write(reinterpret_cast<const char*>(content.value()), content.value_size());
  m_totalSize += content.value_size();
  if (m_verbose) {
    std::cerr << "LOG: received data = " << data.getName() << std::endl;
  }
  if (m_isFinished || m_isSingle) {
    std::cerr << "INFO: End of file is reached" << std::endl;
    std::cerr << "INFO: Total # of segments received: " << m_nextSegment  << std::endl;
    std::cerr << "INFO: Total # bytes of content received: " << m_totalSize << std::endl;
  }
}

void
Consumer::fetchNextData(const Name& name, const Data& data)
{
  uint64_t segment = name[-1].toSegment();
  BOOST_VERIFY(segment == (m_nextSegment - 1));

  auto finalBlockId = data.getFinalBlock();
  if (finalBlockId == name[-1]) {
    m_isFinished = true;
  }
  else {
    // Reset retry counter
    m_retryCount = 0;
    if (m_hasVersion)
      fetchData(Name(m_dataName).appendSegment(m_nextSegment++));
    else
      fetchData(Name(m_dataName).append(name[-2]).appendSegment(m_nextSegment++));
  }
}

void
Consumer::onTimeout(const Interest& interest)
{
  if (m_retryCount++ < MAX_RETRY) {
    // Retransmit the interest
    fetchData(interest.getName());
    if (m_verbose) {
      std::cerr << "TIMEOUT: retransmit interest for " << interest.getName() << std::endl;
    }
  }
  else {
    std::cerr << "TIMEOUT: last interest sent for segment #" << (m_nextSegment - 1) << std::endl;
    std::cerr << "TIMEOUT: abort fetching after " << MAX_RETRY
              << " times of retry" << std::endl;
  }
}

static void
usage(const char* programName)
{
  std::cerr << "Usage: "
            << programName << " [-v] [-s] [-u] [-l lifetime] [-w timeout] [-o filename] ndn-name\n"
            << "\n"
            << "  -v: be verbose\n"
            << "  -s: only get single data packet\n"
            << "  -u: versioned: ndn-name contains version component\n"
            << "      if -u is not specified, this command will return the rightmost child for the prefix\n"
            << "  -l: InterestLifetime in milliseconds\n"
            << "  -w: timeout in milliseconds for whole process (default unlimited)\n"
            << "  -o: write to local file name instead of stdout\n"
            << "  ndn-name: NDN Name prefix for Data to be read\n"
            << std::endl;
}

static int
main(int argc, char** argv)
{
  std::string name;
  const char* outputFile = nullptr;
  bool verbose = false, versioned = false, single = false;
  int interestLifetime = 4000;  // in milliseconds
  int timeout = 0;  // in milliseconds

  int opt;
  while ((opt = getopt(argc, argv, "hvsul:w:o:")) != -1) {
    switch (opt) {
    case 'h':
      usage(argv[0]);
      return 0;
    case 'v':
      verbose = true;
      break;
    case 's':
      single = true;
      break;
    case 'u':
      versioned = true;
      break;
    case 'l':
      try {
        interestLifetime = boost::lexical_cast<int>(optarg);
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "ERROR: -l option should be an integer" << std::endl;
        return 2;
      }
      interestLifetime = std::max(interestLifetime, 0);
      break;
    case 'w':
      try {
        timeout = boost::lexical_cast<int>(optarg);
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "ERROR: -w option should be an integer" << std::endl;
        return 2;
      }
      timeout = std::max(timeout, 0);
      break;
    case 'o':
      outputFile = optarg;
      break;
    default:
      usage(argv[0]);
      return 2;
    }
  }

  if (optind < argc) {
    name = argv[optind];
  }

  if (name.empty()) {
    usage(argv[0]);
    return 2;
  }

  std::streambuf* buf;
  std::ofstream of;

  if (outputFile != nullptr) {
    of.open(outputFile, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!of || !of.is_open()) {
      std::cerr << "ERROR: cannot open " << outputFile << std::endl;
      return 2;
    }
    buf = of.rdbuf();
  }
  else {
    buf = std::cout.rdbuf();
  }

  std::ostream os(buf);
  Consumer consumer(name, os, verbose, versioned, single, interestLifetime, timeout);

  try {
    consumer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

} // namespace repo

int
main(int argc, char** argv)
{
  return repo::main(argc, argv);
}
