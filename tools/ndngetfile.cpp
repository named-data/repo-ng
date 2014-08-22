/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
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

#include "ndngetfile.hpp"
#include <boost/lexical_cast.hpp>
#include <fstream>

namespace repo {

using namespace ndn;

static const int MAX_RETRY = 3;

void
Consumer::fetchData(const Name& name)
{
  Interest interest(name);
  interest.setInterestLifetime(m_interestLifetime);
  //std::cout<<"interest name = "<<interest.getName()<<std::endl;
  if (m_hasVersion)
    {
      interest.setMustBeFresh(m_mustBeFresh);
    }
  else
    {
      interest.setMustBeFresh(true);
      interest.setChildSelector(1);
    }

  m_face.expressInterest(interest,
                         m_hasVersion ?
                           bind(&Consumer::onVersionedData, this, _1, _2)
                           :
                           bind(&Consumer::onUnversionedData, this, _1, _2),
                         bind(&Consumer::onTimeout, this, _1));
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
Consumer::onVersionedData(const Interest& interest, Data& data)
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
Consumer::onUnversionedData(const Interest& interest, Data& data)
{
  const Name& name = data.getName();
  //std::cout<<"recevied data name = "<<name<<std::endl;
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
  const Block& content = data.getContent();
  m_os.write(reinterpret_cast<const char*>(content.value()), content.value_size());
  m_totalSize += content.value_size();
  if (m_verbose)
  {
    std::cerr << "LOG: received data = " << data.getName() << std::endl;
  }
  if (m_isFinished || m_isSingle) {
    std::cerr << "INFO: End of file is reached." << std::endl;
    std::cerr << "INFO: Total # of segments received: " << m_nextSegment  << std::endl;
    std::cerr << "INFO: Total # bytes of content received: " << m_totalSize << std::endl;
  }
}

void
Consumer::fetchNextData(const Name& name, const Data& data)
{
  uint64_t segment = name[-1].toSegment();
  BOOST_ASSERT(segment == (m_nextSegment - 1));
  const name::Component& finalBlockId = data.getMetaInfo().getFinalBlockId();
  if (finalBlockId == name[-1]) {
    m_isFinished = true;
  }
  else
  {
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
  if (m_retryCount++ < MAX_RETRY)
    {
      // Retransmit the interest
      fetchData(interest.getName());
      if (m_verbose)
        {
          std::cerr << "TIMEOUT: retransmit interest for " << interest.getName() << std::endl;
        }
    }
  else
    {
      std::cerr << "TIMEOUT: last interest sent for segment #" << (m_nextSegment - 1) << std::endl;
      std::cerr << "TIMEOUT: abort fetching after " << MAX_RETRY
                << " times of retry" << std::endl;
    }
}


int
usage(const std::string& filename)
{
  std::cerr << "Usage: \n    "
            << filename << " [-v] [-s] [-u] [-l lifetime] [-w timeout] [-o filename] ndn-name\n\n"
            << "-v: be verbose\n"
            << "-s: only get single data packet\n"
            << "-u: versioned: ndn-name contains version component\n"
            << "    if -u is not specified, this command will return the rightmost child for the prefix\n"
            << "-l: InterestLifetime in milliseconds\n"
            << "-w: timeout in milliseconds for whole process (default unlimited)\n"
            << "-o: write to local file name instead of stdout\n"
            << "ndn-name: NDN Name prefix for Data to be read\n";
  return 1;
}


int
main(int argc, char** argv)
{
  std::string name;
  const char* outputFile = 0;
  bool verbose = false, versioned = false, single = false;
  int interestLifetime = 4000;  // in milliseconds
  int timeout = 0;  // in milliseconds

  int opt;
  while ((opt = getopt(argc, argv, "vsul:w:o:")) != -1)
    {
      switch (opt) {
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
        try
          {
            interestLifetime = boost::lexical_cast<int>(optarg);
          }
        catch (boost::bad_lexical_cast&)
          {
            std::cerr << "ERROR: -l option should be an integer." << std::endl;
            return 1;
          }
        interestLifetime = std::max(interestLifetime, 0);
        break;
      case 'w':
        try
          {
            timeout = boost::lexical_cast<int>(optarg);
          }
        catch (boost::bad_lexical_cast&)
          {
            std::cerr << "ERROR: -w option should be an integer." << std::endl;
            return 1;
          }
        timeout = std::max(timeout, 0);
        break;
      case 'o':
        outputFile = optarg;
        break;
      default:
        return usage(argv[0]);
      }
    }

  if (optind < argc)
    {
      name = argv[optind];
    }

  if (name.empty())
    {
      return usage(argv[0]);
    }

  std::streambuf* buf;
  std::ofstream of;

  if (outputFile != 0)
    {
      of.open(outputFile);
      if (!of)
        {
          std::cerr << "ERROR: output file is invalid" << std::endl;
          return 1;
        }
      buf = of.rdbuf();
    }
  else
    {
      buf = std::cout.rdbuf();
    }

  std::ostream os(buf);

  Consumer consumer(name, os, verbose, versioned, single,
                    interestLifetime, timeout);

  try
    {
      consumer.run();
    }
  catch (const std::exception& e)
    {
      std::cerr << "ERROR: " << e.what() << std::endl;
    }

  return 0;
}

} // namespace repo

int
main(int argc, char** argv)
{
  return repo::main(argc, argv);
}
