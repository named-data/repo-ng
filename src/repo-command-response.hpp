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

#ifndef REPO_REPO_COMMAND_RESPONSE_HPP
#define REPO_REPO_COMMAND_RESPONSE_HPP

#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>
#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/encoding/tlv-nfd.hpp>
#include "repo-tlv.hpp"

namespace repo {

using ndn::Block;
using ndn::EncodingImpl;
using ndn::EncodingEstimator;
using ndn::EncodingBuffer;

/**
* @brief Class defining abstraction of Response for NDN Repo Protocol
* @sa link http://redmine.named-data.net/projects/repo-ng/wiki/Repo_Protocol_Specification#Repo-Command-Response
*/
class RepoCommandResponse
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

  RepoCommandResponse()
    : m_hasStartBlockId(false)
    , m_hasEndBlockId(false)
    , m_hasProcessId(false)
    , m_hasInsertNum(false)
    , m_hasDeleteNum(false)
    , m_hasStatusCode(false)
  {
  }

  explicit
  RepoCommandResponse(const Block& block)
  {
    wireDecode(block);
  }

  uint64_t
  getStartBlockId() const
  {
    return m_startBlockId;
  }

  RepoCommandResponse&
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

  RepoCommandResponse&
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
    return m_processId;
  }

  RepoCommandResponse&
  setProcessId(uint64_t processId)
  {
    m_processId  = processId;
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
  getStatusCode() const
  {
    return m_statusCode;
  }

  RepoCommandResponse&
  setStatusCode(uint64_t statusCode)
  {
    m_statusCode  = statusCode;
    m_hasStatusCode = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasStatusCode() const
  {
    return m_hasStatusCode;
  }

  uint64_t
  getInsertNum() const
  {
    return m_insertNum;
  }

  RepoCommandResponse&
  setInsertNum(uint64_t insertNum)
  {
    m_insertNum = insertNum;
    m_hasInsertNum = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasInsertNum() const
  {
    return m_hasInsertNum;
  }

  uint64_t
  getDeleteNum() const
  {
    return m_deleteNum;
  }

  RepoCommandResponse&
  setDeleteNum(uint64_t deleteNum)
  {
    m_deleteNum = deleteNum;
    m_hasDeleteNum = true;
    m_wire.reset();
    return *this;
  }

  bool
  hasDeleteNum() const
  {
    return m_hasDeleteNum;
  }

  template<bool T>
  size_t
  wireEncode(EncodingImpl<T>& block) const;

  const Block&
  wireEncode() const;

  void
  wireDecode(const Block& wire);

private:
  uint64_t m_statusCode;
  uint64_t m_startBlockId;
  uint64_t m_endBlockId;
  uint64_t m_processId;
  uint64_t m_insertNum;
  uint64_t m_deleteNum;

  bool m_hasStartBlockId;
  bool m_hasEndBlockId;
  bool m_hasProcessId;
  bool m_hasInsertNum;
  bool m_hasDeleteNum;
  bool m_hasStatusCode;

  mutable Block m_wire;
};

template<bool T>
inline size_t
RepoCommandResponse::wireEncode(EncodingImpl<T>& encoder) const
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
    variableLength = encoder.prependNonNegativeInteger(m_statusCode);
    totalLength += variableLength;
    totalLength += encoder.prependVarNumber(variableLength);
    totalLength += encoder.prependVarNumber(tlv::StatusCode);
  } else {
    throw Error("required field StatusCode is missing");
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

inline const Block&
RepoCommandResponse::wireEncode() const
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
RepoCommandResponse::wireDecode(const Block& wire)
{
  m_hasStartBlockId = false;
  m_hasEndBlockId = false;
  m_hasProcessId = false;
  m_hasStatusCode = false;
  m_hasInsertNum = false;
  m_hasDeleteNum = false;

  m_wire = wire;

  m_wire.parse();

  Block::element_const_iterator val;

  if (m_wire.type() != tlv::RepoCommandResponse)
    throw Error("RepoCommandResponse malformed");

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

  // StatusCode
  val = m_wire.find(tlv::StatusCode);
  if (val != m_wire.elements_end())
  {
    m_hasStatusCode = true;
    m_statusCode = readNonNegativeInteger(*val);

  } else {
    throw Error("required field StatusCode is missing");
  }

  // InsertNum
  val = m_wire.find(tlv::InsertNum);
  if (val != m_wire.elements_end())
  {
    m_hasInsertNum = true;
    m_insertNum = readNonNegativeInteger(*val);
  }

  // DeleteNum
  val = m_wire.find(tlv::DeleteNum);
  if (val != m_wire.elements_end())
  {
    m_hasDeleteNum = true;
    m_deleteNum = readNonNegativeInteger(*val);
  }
}

inline std::ostream&
operator<<(std::ostream& os, const RepoCommandResponse& repoCommandResponse)
{
  os << "RepoCommandResponse(";

  if (repoCommandResponse.hasProcessId()) {
    os << " ProcessId: " << repoCommandResponse.getProcessId();
  }
  if (repoCommandResponse.hasStatusCode()) {
    os << " StatusCode: " << repoCommandResponse.getStatusCode();
  }
  if (repoCommandResponse.hasStartBlockId()) {
    os << " StartBlockId: " << repoCommandResponse.getStartBlockId();
  }
  if (repoCommandResponse.hasEndBlockId()) {
    os << " EndBlockId: " << repoCommandResponse.getEndBlockId();
  }
  if (repoCommandResponse.hasInsertNum()) {
    os << " InsertNum: " << repoCommandResponse.getInsertNum();
  }
  if (repoCommandResponse.hasDeleteNum()) {
    os << " DeleteNum: " << repoCommandResponse.getDeleteNum();

  }
  os << " )";
  return os;
}

} // namespace repo

#endif // REPO_REPO_COMMAND_RESPONSE_HPP
