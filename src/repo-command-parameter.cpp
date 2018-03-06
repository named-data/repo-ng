/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "repo-command-parameter.hpp"

#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/mgmt/control-parameters.hpp>
#include <ndn-cxx/name.hpp>

namespace repo {

RepoCommandParameter&
RepoCommandParameter::setName(const Name& name)
{
  m_name = name;
  m_hasFields[REPO_PARAMETER_NAME] = true;
  m_wire.reset();
  return *this;
}

RepoCommandParameter&
RepoCommandParameter::setStartBlockId(uint64_t startBlockId)
{
  m_startBlockId  = startBlockId;
  m_hasFields[REPO_PARAMETER_START_BLOCK_ID] = true;
  m_wire.reset();
  return *this;
}

RepoCommandParameter&
RepoCommandParameter::setEndBlockId(uint64_t endBlockId)
{
  m_endBlockId  = endBlockId;
  m_hasFields[REPO_PARAMETER_END_BLOCK_ID] = true;
  m_wire.reset();
  return *this;
}

RepoCommandParameter&
RepoCommandParameter::setProcessId(uint64_t processId)
{
  m_processId = processId;
  m_hasFields[REPO_PARAMETER_PROCESS_ID] = true;
  m_wire.reset();
  return *this;
}

RepoCommandParameter&
RepoCommandParameter::setInterestLifetime(milliseconds interestLifetime)
{
  m_interestLifetime = interestLifetime;
  m_hasFields[REPO_PARAMETER_INTEREST_LIFETIME] = true;
  m_wire.reset();
  return *this;
}

template<ndn::encoding::Tag T>
size_t
RepoCommandParameter::wireEncode(EncodingImpl<T>& encoder) const
{
  size_t totalLength = 0;
  size_t variableLength = 0;

  if (m_hasFields[REPO_PARAMETER_PROCESS_ID]) {
    variableLength = encoder.prependNonNegativeInteger(m_processId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::ProcessId);
  }

  if (m_hasFields[REPO_PARAMETER_END_BLOCK_ID]) {
    variableLength = encoder.prependNonNegativeInteger(m_endBlockId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::EndBlockId);
  }

  if (m_hasFields[REPO_PARAMETER_START_BLOCK_ID]) {
    variableLength = encoder.prependNonNegativeInteger(m_startBlockId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::StartBlockId);
  }

  if (m_hasFields[REPO_PARAMETER_INTEREST_LIFETIME]) {
    variableLength = encoder.prependNonNegativeInteger(m_interestLifetime.count());
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::InterestLifetime);
  }

  if (m_hasFields[REPO_PARAMETER_NAME]) {
    totalLength += getName().wireEncode(encoder);
  }

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::RepoCommandParameter);
  return totalLength;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RepoCommandParameter);

Block
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

void
RepoCommandParameter::wireDecode(const Block& wire)
{
  m_wire = wire;

  m_wire.parse();

  if (m_wire.type() != tlv::RepoCommandParameter)
    BOOST_THROW_EXCEPTION(Error("Requested decoding of RepoCommandParameter, but Block is of different type"));

  // Name
  Block::element_const_iterator val = m_wire.find(tlv::Name);
  if (val != m_wire.elements_end())
  {
    m_hasFields[REPO_PARAMETER_NAME] = true;
    m_name.wireDecode(m_wire.get(tlv::Name));
  }

  // StartBlockId
  val = m_wire.find(tlv::StartBlockId);
  if (val != m_wire.elements_end())
  {
    m_hasFields[REPO_PARAMETER_START_BLOCK_ID] = true;
    m_startBlockId = readNonNegativeInteger(*val);
  }

  // EndBlockId
  val = m_wire.find(tlv::EndBlockId);
  if (val != m_wire.elements_end())
  {
    m_hasFields[REPO_PARAMETER_END_BLOCK_ID] = true;
    m_endBlockId = readNonNegativeInteger(*val);
  }

  // ProcessId
  val = m_wire.find(tlv::ProcessId);
  if (val != m_wire.elements_end())
  {
    m_hasFields[REPO_PARAMETER_PROCESS_ID] = true;
    m_processId = readNonNegativeInteger(*val);
  }

  // InterestLifeTime
  val = m_wire.find(tlv::InterestLifetime);
  if (val != m_wire.elements_end())
  {
    m_hasFields[REPO_PARAMETER_INTEREST_LIFETIME] = true;
    m_interestLifetime = milliseconds(readNonNegativeInteger(*val));
  }
}

std::ostream&
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
  // InterestLifetime
  if (repoCommandParameter.hasProcessId()) {
    os << " InterestLifetime: " << repoCommandParameter.getInterestLifetime();
  }
  os << " )";
  return os;
}

} // namespace repo