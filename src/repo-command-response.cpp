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

#include "repo-command-response.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>

namespace repo {

RepoCommandResponse&
RepoCommandResponse::setStartBlockId(uint64_t startBlockId)
{
  m_startBlockId  = startBlockId;
  m_hasStartBlockId = true;
  m_wire.reset();
  return *this;
}

bool
RepoCommandResponse::hasStartBlockId() const
{
  return m_hasStartBlockId;
}

RepoCommandResponse&
RepoCommandResponse::setEndBlockId(uint64_t endBlockId)
{
  m_endBlockId  = endBlockId;
  m_hasEndBlockId = true;
  m_wire.reset();
  return *this;
}

bool
RepoCommandResponse::hasEndBlockId() const
{
  return m_hasEndBlockId;
}

RepoCommandResponse&
RepoCommandResponse::setProcessId(uint64_t processId)
{
  m_processId  = processId;
  m_hasProcessId = true;
  m_wire.reset();
  return *this;
}

bool
RepoCommandResponse::hasProcessId() const
{
  return m_hasProcessId;
}

RepoCommandResponse&
RepoCommandResponse::setCode(uint32_t statusCode)
{
  m_hasStatusCode = true;
  auto* response = static_cast<RepoCommandResponse*>(&ndn::mgmt::ControlResponse::setCode(statusCode));
  return *response;
}

bool
RepoCommandResponse::hasStatusCode() const
{
  return m_hasStatusCode;
}

RepoCommandResponse&
RepoCommandResponse::setInsertNum(uint64_t insertNum)
{
  m_insertNum = insertNum;
  m_hasInsertNum = true;
  m_wire.reset();
  return *this;
}

bool
RepoCommandResponse::hasInsertNum() const
{
  return m_hasInsertNum;
}

RepoCommandResponse&
RepoCommandResponse::setDeleteNum(uint64_t deleteNum)
{
  m_deleteNum = deleteNum;
  m_hasDeleteNum = true;
  m_wire.reset();
  return *this;
}

bool
RepoCommandResponse::hasDeleteNum() const
{
  return m_hasDeleteNum;
}

const Block&
RepoCommandResponse::wireEncode() const
{
  if (m_wire.hasWire())
    return m_wire;

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();
  return m_wire;
}

template<ndn::encoding::Tag T>
size_t
RepoCommandResponse::wireEncode(ndn::EncodingImpl<T>& encoder) const
{
  size_t totalLength = 0;
  size_t variableLength = 0;

  if (m_hasDeleteNum) {
    variableLength = encoder.prependNonNegativeInteger(m_deleteNum);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::DeleteNum);
  }

  if (m_hasInsertNum) {
    variableLength = encoder.prependNonNegativeInteger(m_insertNum);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::InsertNum);
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
    totalLength += encoder.prependVarNumber(repo::tlv::StartBlockId);
  }

  if (m_hasStatusCode) {
    variableLength = encoder.prependNonNegativeInteger(getCode());
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::StatusCode);
  }
  else {
    NDN_THROW(Error("Required field StatusCode is missing"));
  }

  if (m_hasProcessId) {
    variableLength = encoder.prependNonNegativeInteger(m_processId);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::ProcessId);
  }

  totalLength += encoder.prependVarNumber(totalLength);
  totalLength += encoder.prependVarNumber(tlv::RepoCommandResponse);
  return totalLength;
}

void
RepoCommandResponse::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::RepoCommandResponse) {
    NDN_THROW(Error("RepoCommandResponse", wire.type()));
  }

  m_hasStartBlockId = false;
  m_hasEndBlockId = false;
  m_hasProcessId = false;
  m_hasStatusCode = false;
  m_hasInsertNum = false;
  m_hasDeleteNum = false;

  m_wire = wire;
  m_wire.parse();

  // StartBlockId
  auto val = m_wire.find(tlv::StartBlockId);
  if (val != m_wire.elements_end()) {
    m_hasStartBlockId = true;
    m_startBlockId = readNonNegativeInteger(*val);
  }

  // EndBlockId
  val = m_wire.find(tlv::EndBlockId);
  if (val != m_wire.elements_end()) {
    m_hasEndBlockId = true;
    m_endBlockId = readNonNegativeInteger(*val);
  }

  // ProcessId
  val = m_wire.find(tlv::ProcessId);
  if (val != m_wire.elements_end()) {
    m_hasProcessId = true;
    m_processId = readNonNegativeInteger(*val);
  }

  // StatusCode
  val = m_wire.find(tlv::StatusCode);
  if (val != m_wire.elements_end()) {
    setCode(readNonNegativeInteger(*val));
  }
  else {
    NDN_THROW(Error("Required field StatusCode is missing"));
  }

  // InsertNum
  val = m_wire.find(tlv::InsertNum);
  if (val != m_wire.elements_end()) {
    m_hasInsertNum = true;
    m_insertNum = readNonNegativeInteger(*val);
  }

  // DeleteNum
  val = m_wire.find(tlv::DeleteNum);
  if (val != m_wire.elements_end()) {
    m_hasDeleteNum = true;
    m_deleteNum = readNonNegativeInteger(*val);
  }
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(RepoCommandResponse);

} // namespace repo
