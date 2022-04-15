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

#ifndef REPO_REPO_COMMAND_RESPONSE_HPP
#define REPO_REPO_COMMAND_RESPONSE_HPP

#include "common.hpp"
#include "repo-tlv.hpp"

#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/mgmt/control-response.hpp>

namespace repo {

/**
 * @brief Class defining abstraction of Response for NDN Repo Protocol
 * @sa link https://redmine.named-data.net/projects/repo-ng/wiki/Repo_Protocol_Specification#Repo-Command-Response
 */
class RepoCommandResponse : public ndn::mgmt::ControlResponse
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  RepoCommandResponse() = default;

  RepoCommandResponse(uint32_t code, const std::string& text)
    : ndn::mgmt::ControlResponse(code, text)
    , m_hasStartBlockId(false)
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
  setStartBlockId(uint64_t startBlockId);

  bool
  hasStartBlockId() const;

  uint64_t
  getEndBlockId() const
  {
    BOOST_ASSERT(hasEndBlockId());
    return m_endBlockId;
  }

  RepoCommandResponse&
  setEndBlockId(uint64_t endBlockId);

  bool
  hasEndBlockId() const;

  uint64_t
  getProcessId() const
  {
    return m_processId;
  }

  RepoCommandResponse&
  setProcessId(uint64_t processId);

  bool
  hasProcessId() const;

  RepoCommandResponse&
  setCode(uint32_t statusCode);

  bool
  hasStatusCode() const;

  uint64_t
  getInsertNum() const
  {
    return m_insertNum;
  }

  RepoCommandResponse&
  setInsertNum(uint64_t insertNum);

  bool
  hasInsertNum() const;

  uint64_t
  getDeleteNum() const
  {
    return m_deleteNum;
  }

  RepoCommandResponse&
  setDeleteNum(uint64_t deleteNum);

  bool
  hasDeleteNum() const;

  template<ndn::encoding::Tag T>
  size_t
  wireEncode(ndn::EncodingImpl<T>& block) const;

  const Block&
  wireEncode() const;

  void
  wireDecode(const Block& wire);

private:
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

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(RepoCommandResponse);

} // namespace repo

#endif // REPO_REPO_COMMAND_RESPONSE_HPP
