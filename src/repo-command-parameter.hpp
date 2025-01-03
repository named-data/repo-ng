/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025, Regents of the University of California.
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

#include "common.hpp"
#include "repo-tlv.hpp"

#include <ndn-cxx/encoding/encoding-buffer.hpp>
#include <ndn-cxx/mgmt/control-parameters-base.hpp>

namespace repo {

enum RepoParameterField {
  REPO_PARAMETER_NAME,
  REPO_PARAMETER_START_BLOCK_ID,
  REPO_PARAMETER_END_BLOCK_ID,
  REPO_PARAMETER_PROCESS_ID,
  REPO_PARAMETER_INTEREST_LIFETIME,
  REPO_PARAMETER_UBOUND
};

const std::string REPO_PARAMETER_FIELD[REPO_PARAMETER_UBOUND] = {
  "Name",
  "StartBlockId",
  "EndBlockId",
  "ProcessId",
  "InterestLifetime"
};

/**
 * @brief Class defining abstraction of parameter of command for NDN Repo Protocol
 * @sa link https://redmine.named-data.net/projects/repo-ng/wiki/Repo_Protocol_Specification#RepoCommandParameter
 */
class RepoCommandParameter : public ndn::mgmt::ControlParametersBase
{
public:
  class Error : public tlv::Error
  {
  public:
    using tlv::Error::Error;
  };

  RepoCommandParameter()
    : m_hasFields(REPO_PARAMETER_UBOUND)
  {
  }

  explicit
  RepoCommandParameter(const Block& block)
    : m_hasFields(REPO_PARAMETER_UBOUND)
  {
    wireDecode(block);
  }

  const Name&
  getName() const
  {
    return m_name;
  }

  RepoCommandParameter&
  setName(const Name& name);

  bool
  hasName() const
  {
    return m_hasFields[REPO_PARAMETER_NAME];
  }


  uint64_t
  getStartBlockId() const
  {
    BOOST_ASSERT(hasStartBlockId());
    return m_startBlockId;
  }

  RepoCommandParameter&
  setStartBlockId(uint64_t startBlockId);

  bool
  hasStartBlockId() const
  {
    return m_hasFields[REPO_PARAMETER_START_BLOCK_ID];
  }

  uint64_t
  getEndBlockId() const
  {
    BOOST_ASSERT(hasEndBlockId());
    return m_endBlockId;
  }

  RepoCommandParameter&
  setEndBlockId(uint64_t endBlockId);

  bool
  hasEndBlockId() const
  {
    return m_hasFields[REPO_PARAMETER_END_BLOCK_ID];
  }

  uint64_t
  getProcessId() const
  {
    BOOST_ASSERT(hasProcessId());
    return m_processId;
  }

  RepoCommandParameter&
  setProcessId(uint64_t processId);

  bool
  hasProcessId() const
  {
    return m_hasFields[REPO_PARAMETER_PROCESS_ID];
  }

  time::milliseconds
  getInterestLifetime() const
  {
    BOOST_ASSERT(hasInterestLifetime());
    return m_interestLifetime;
  }

  RepoCommandParameter&
  setInterestLifetime(time::milliseconds interestLifetime);

  bool
  hasInterestLifetime() const
  {
    return m_hasFields[REPO_PARAMETER_INTEREST_LIFETIME];
  }

  const std::vector<bool>&
  getPresentFields() const
  {
    return m_hasFields;
  }

  template<ndn::encoding::Tag T>
  size_t
  wireEncode(ndn::EncodingImpl<T>& block) const;

  Block
  wireEncode() const override;

  void
  wireDecode(const Block& wire) override;

private:
  std::vector<bool> m_hasFields;
  Name m_name;
  uint64_t m_startBlockId;
  uint64_t m_endBlockId;
  uint64_t m_processId;
  time::milliseconds m_interestLifetime;

  mutable Block m_wire;
};

NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(RepoCommandParameter);

} // namespace repo

#endif // REPO_REPO_COMMAND_PARAMETER_HPP
