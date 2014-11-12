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

#ifndef REPO_REPO_COMMAND_PARAMETER_HPP
#define REPO_REPO_COMMAND_PARAMETER_HPP

#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/selectors.hpp>
#include "repo-tlv.hpp"

namespace repo {

using ndn::Name;
using ndn::Block;
using ndn::EncodingImpl;
using ndn::Selectors;
using ndn::EncodingEstimator;
using ndn::EncodingBuffer;
using namespace ndn::time;

/**
* @brief Class defining abstraction of parameter of command for NDN Repo Protocol
* @sa link http://redmine.named-data.net/projects/repo-ng/wiki/Repo_Protocol_Specification#RepoCommandParameter
**/

class RepoCommandParameter
{
public:
  class Error : public ndn::tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : ndn::tlv::Error(what)
    {
    }
  };

  RepoCommandParameter()
    : m_hasName(false)
    , m_hasStartBlockId(false)
    , m_hasEndBlockId(false)
    , m_hasProcessId(false)
    , m_hasMaxInterestNum(false)
    , m_hasWatchTimeout(false)
    , m_hasInterestLifetime(false)
  {
  }

  explicit
  RepoCommandParameter(const Block& block)
  {
    wireDecode(block);
  }

  const Name&
  getName() const
  {
    return m_name;
  }

  RepoCommandParameter&
  setName(const Name& name)
  {
    m_name = name;
    m_hasName = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasName() const
  {
    return m_hasName;
  }

  const Selectors&
  getSelectors() const
  {
    return m_selectors;
  }

  RepoCommandParameter&
  setSelectors(const Selectors& selectors)
  {
    m_selectors = selectors;
    m_wire.reset();
    return *this;
  }

  bool
  hasSelectors() const
  {
    return !m_selectors.empty();
  }

  uint64_t
  getStartBlockId() const
  {
    assert(hasStartBlockId());
    return m_startBlockId;
  }

  RepoCommandParameter&
  setStartBlockId(uint64_t startBlockId)
  {
    m_startBlockId  = startBlockId;
    m_hasStartBlockId = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasStartBlockId() const
  {
    return m_hasStartBlockId;
  }

  uint64_t
  getEndBlockId() const
  {
    assert(hasEndBlockId());
    return m_endBlockId;
  }

  RepoCommandParameter&
  setEndBlockId(uint64_t endBlockId)
  {
    m_endBlockId  = endBlockId;
    m_hasEndBlockId = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasEndBlockId() const
  {
    return m_hasEndBlockId;
  }

  uint64_t
  getProcessId() const
  {
    assert(hasProcessId());
    return m_processId;
  }

  RepoCommandParameter&
  setProcessId(uint64_t processId)
  {
    m_processId = processId;
    m_hasProcessId = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasProcessId() const
  {
    return m_hasProcessId;
  }

  uint64_t
  getMaxInterestNum() const
  {
    assert(hasMaxInterestNum());
    return m_maxInterestNum;
  }

  RepoCommandParameter&
  setMaxInterestNum(uint64_t maxInterestNum)
  {
    m_maxInterestNum = maxInterestNum;
    m_hasMaxInterestNum = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasMaxInterestNum() const
  {
    return m_hasMaxInterestNum;
  }

  milliseconds
  getWatchTimeout() const
  {
    assert(hasWatchTimeout());
    return m_watchTimeout;
  }

  RepoCommandParameter&
  setWatchTimeout(milliseconds watchTimeout)
  {
    m_watchTimeout = watchTimeout;
    m_hasWatchTimeout = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasWatchTimeout() const
  {
    return m_hasWatchTimeout;
  }

  milliseconds
  getInterestLifetime() const
  {
    assert(hasInterestLifetime());
    return m_interestLifetime;
  }

  RepoCommandParameter&
  setInterestLifetime(milliseconds interestLifetime)
  {
    m_interestLifetime = interestLifetime;
    m_hasInterestLifetime = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasInterestLifetime() const
  {
    return m_hasInterestLifetime;
  }

  template<bool T>
  size_t
  wireEncode(EncodingImpl<T>& block) const;

  const Block&
  wireEncode() const;

  void
  wireDecode(const Block& wire);

private:

  Name m_name;
  Selectors m_selectors;
  uint64_t m_startBlockId;
  uint64_t m_endBlockId;
  uint64_t m_processId;
  uint64_t m_maxInterestNum;
  milliseconds m_watchTimeout;
  milliseconds m_interestLifetime;

  bool m_hasName;
  bool m_hasStartBlockId;
  bool m_hasEndBlockId;
  bool m_hasProcessId;
  bool m_hasMaxInterestNum;
  bool m_hasWatchTimeout;
  bool m_hasInterestLifetime;

  mutable Block m_wire;
};

template<bool T>
inline size_t
RepoCommandParameter::wireEncode(EncodingImpl<T>& encoder) const
{
  size_t totalLength = 0;
  size_t variableLength = 0;

  if (m_hasProcessId) {
    variableLength = encoder.prependNonNegativeInteger(m_processId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::ProcessId);
  }

  if (m_hasEndBlockId) {
    variableLength = encoder.prependNonNegativeInteger(m_endBlockId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::EndBlockId);
  }

  if (m_hasStartBlockId) {
    variableLength = encoder.prependNonNegativeInteger(m_startBlockId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::StartBlockId);
  }

  if (m_hasMaxInterestNum) {
    variableLength = encoder.prependNonNegativeInteger(m_maxInterestNum);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::MaxInterestNum);
  }

  if (m_hasWatchTimeout) {
    variableLength = encoder.prependNonNegativeInteger(m_watchTimeout.count());
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::WatchTimeout);
  }

  if (m_hasInterestLifetime) {
    variableLength = encoder.prependNonNegativeInteger(m_interestLifetime.count());
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::InterestLifetime);
  }

  if (!getSelectors().empty()) {
    totalLength += getSelectors().wireEncode(encoder);
  }

  if (m_hasName) {
    totalLength += getName().wireEncode(encoder);
  }

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::RepoCommandParameter);
  return totalLength;
}

inline const Block&
RepoCommandParameter::wireEncode() const
{
  if (m_wire.hasWire())
    return m_wire;

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();
  return m_wire;
}

inline void
RepoCommandParameter::wireDecode(const Block& wire)
{
  m_hasName = false;
  m_hasStartBlockId = false;
  m_hasEndBlockId = false;
  m_hasProcessId = false;
  m_hasMaxInterestNum = false;
  m_hasWatchTimeout = false;
  m_hasInterestLifetime = false;

  m_wire = wire;

  m_wire.parse();

  if (m_wire.type() != tlv::RepoCommandParameter)
    throw Error("Requested decoding of RepoCommandParameter, but Block is of different type");

  // Name
  Block::element_const_iterator val = m_wire.find(tlv::Name);
  if (val != m_wire.elements_end())
  {
    m_hasName = true;
    m_name.wireDecode(m_wire.get(tlv::Name));
  }

  // Selectors
  val = m_wire.find(tlv::Selectors);
  if (val != m_wire.elements_end())
  {
    m_selectors.wireDecode(*val);
  }
  else
    m_selectors = Selectors();

  // StartBlockId
  val = m_wire.find(tlv::StartBlockId);
  if (val != m_wire.elements_end())
  {
    m_hasStartBlockId = true;
    m_startBlockId = readNonNegativeInteger(*val);
  }

  // EndBlockId
  val = m_wire.find(tlv::EndBlockId);
  if (val != m_wire.elements_end())
  {
    m_hasEndBlockId = true;
    m_endBlockId = readNonNegativeInteger(*val);
  }

  // ProcessId
  val = m_wire.find(tlv::ProcessId);
  if (val != m_wire.elements_end())
  {
    m_hasProcessId = true;
    m_processId = readNonNegativeInteger(*val);
  }

  // MaxInterestNum
  val = m_wire.find(tlv::MaxInterestNum);
  if (val != m_wire.elements_end())
  {
    m_hasMaxInterestNum = true;
    m_maxInterestNum = readNonNegativeInteger(*val);
  }

  // WatchTimeout
  val = m_wire.find(tlv::WatchTimeout);
  if (val != m_wire.elements_end())
  {
    m_hasWatchTimeout = true;
    m_watchTimeout = milliseconds(readNonNegativeInteger(*val));
  }

  // InterestLiftTime
  val = m_wire.find(tlv::InterestLifetime);
  if (val != m_wire.elements_end())
  {
    m_hasInterestLifetime = true;
    m_interestLifetime = milliseconds(readNonNegativeInteger(*val));
  }


}

inline std::ostream&
operator<<(std::ostream& os, const RepoCommandParameter& repoCommandParameter)
{
  os << "RepoCommandParameter(";

  // Name
  if (repoCommandParameter.hasName()) {
    os << " Name: " << repoCommandParameter.getName();
  }
  if (repoCommandParameter.hasStartBlockId()) {
  // StartBlockId
    os << " StartBlockId: " << repoCommandParameter.getStartBlockId();
  }
  // EndBlockId
  if (repoCommandParameter.hasEndBlockId()) {
    os << " EndBlockId: " << repoCommandParameter.getEndBlockId();
  }
  // ProcessId
  if (repoCommandParameter.hasProcessId()) {
    os << " ProcessId: " << repoCommandParameter.getProcessId();
  }
  // MaxInterestNum
  if (repoCommandParameter.hasMaxInterestNum()) {
    os << " MaxInterestNum: " << repoCommandParameter.getMaxInterestNum();
  }
  // WatchTimeout
  if (repoCommandParameter.hasProcessId()) {
    os << " WatchTimeout: " << repoCommandParameter.getWatchTimeout();
  }
  // InterestLifetime
  if (repoCommandParameter.hasProcessId()) {
    os << " InterestLifetime: " << repoCommandParameter.getInterestLifetime();
  }
  os << " )";
  return os;
}

} // namespace repo

#endif // REPO_REPO_COMMAND_PARAMETER_HPP
