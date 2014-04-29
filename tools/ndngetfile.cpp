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

namespace repo {

using namespace ndn;

void
Consumer::fetchData(const Name& name)
{
  Interest interest(name);
  interest.setInterestLifetime(m_interestLifetime);

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
                         bind(&Consumer::onData, this, _1, _2),
                         bind(&Consumer::onTimeout, this, _1));
}

void
Consumer::run()
{
  // Send the first Interest
  Name name(m_dataName);
  if (m_hasVersion)
    // If '-u' is specified, we already have version number in the name
    name.appendSegment(m_nextSegment++);

  fetchData(name);

  // processEvents will block until the requested data received or timeout occurs
  m_face.processEvents(m_timeout);
}

void
Consumer::onData(const Interest& interest, Data& data)
{
  const Name& name = data.getName();
  if (name.size() != m_dataName.size() + (m_hasVersion ? 1 : 2))
    throw std::invalid_argument("unexpected data name size.");

  uint64_t segment = name[-1].toSegment();

  if (!m_hasVersion)
    {
      // Assume the second to last component is the version number
      // and the last component is the segment number
      m_dataName.append(name[-2]);
      m_hasVersion = true;
      if (m_verbose)
        {
          std::cerr << "LOG: version number is " << name[-2].toVersion() << std::endl;
        }

      if (segment != 0)
        {
          // Discard this segment and fetch the first segment
          fetchData(Name(m_dataName).appendSegment(m_nextSegment++));
          return;
        }

      m_nextSegment++;
    }

  BOOST_ASSERT(segment == (m_nextSegment - 1));

  // Output the current segment
  const Block& content = data.getContent();
  m_os.write(reinterpret_cast<const char*>(content.value()), content.value_size());
  m_totalSize += content.value_size();
  if (m_verbose)
    {
      std::cerr << "LOG: received segment #" << segment << std::endl;
    }

  // Check final block id
  const name::Component& finalBlockId = data.getMetaInfo().getFinalBlockId();
  if (finalBlockId == name[-1])
    {
      // Reach EOF
      std::cerr << "INFO: End of file is reached." << std::endl;
      std::cerr << "INFO: Total # of segments received: " << (m_nextSegment - 1) << std::endl;
      std::cerr << "INFO: Total # bytes of content received: " << m_totalSize << std::endl;
    }
  else
    {
      // Reset retry counter
      m_retryCount = 0;

      // Fetch next segment
      fetchData(Name(m_dataName).appendSegment(m_nextSegment++));
    }
}

const int Consumer::MAX_RETRY = 3;

void
Consumer::onTimeout(const Interest& interest)
{
  if (m_retryCount++ < Consumer::MAX_RETRY)
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
      std::cerr << "TIMEOUT: abort fetching after " << Consumer::MAX_RETRY
                << " times of retry" << std::endl;
    }
}


int
usage(const std::string& filename)
{
  std::cerr << "Usage: \n    "
            << filename << " [-v] [-u] [-l lifetime] [-w timeout] [-o filename] ndn-name\n\n"
            << "-v: be verbose\n"
            << "-u: unversioned: do not try to find the latest version; "
               "ndn-name contains version component\n"
            << "ndn-name: NDN Name prefix for Data to be read\n"
            << "-l: InterestLifetime in milliseconds\n"
            << "-w: timeout in milliseconds for whole process (default unlimited)\n"
            << "-o: write to local file name instead of stdout\n";
  return 1;
}


int
main(int argc, char** argv)
{
  std::string name;
  const char* outputFile = 0;
  bool verbose = false, unversioned = false;
  int interestLifetime = 4000;  // in milliseconds
  int timeout = 0;  // in milliseconds

  int opt;
  while ((opt = getopt(argc, argv, "vul:w:o:")) != -1)
    {
      switch (opt) {
      case 'v':
        verbose = true;
        break;
      case 'u':
        unversioned = true;
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

  Consumer consumer(name, os, verbose, unversioned,
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
