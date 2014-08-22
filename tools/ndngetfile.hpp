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

#ifndef REPO_NG_TOOLS_NDNGETFILE_HPP
#define REPO_NG_TOOLS_NDNGETFILE_HPP

#include <ndn-cxx/face.hpp>

namespace repo {

class Consumer : boost::noncopyable
{
public:
  Consumer(const std::string& dataName, std::ostream& os,
           bool verbose, bool versioned, bool single,
           int interestLifetime, int timeout,
           bool mustBeFresh = false)
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
  {
  }

  void
  run();

private:
  void
  fetchData(const ndn::Name& name);

  void
  onVersionedData(const ndn::Interest& interest, ndn::Data& data);

  void
  onUnversionedData(const ndn::Interest& interest, ndn::Data& data);

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
};

} // namespace repo

#endif // REPO_NG_TOOLS_NDNGETFILE_HPP
