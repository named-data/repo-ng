/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
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

#include "repo-command.hpp"

namespace repo {

void
RepoCommand::validateRequest(const RepoCommandParameter& parameters) {
  m_requestValidator.validate(parameters);
  check(parameters);
}

void
RepoCommand::FieldValidator::validate(const RepoCommandParameter& parameters) const
{
  const std::vector<bool>& presentFields = parameters.getPresentFields();

  for (size_t i = 0; i < REPO_PARAMETER_UBOUND; i++) {
    bool isPresent = presentFields[i];
    if (m_required[i]) {
      if (!isPresent) {
        BOOST_THROW_EXCEPTION(ArgumentError(REPO_PARAMETER_FIELD[i] + " is required but missing"));
      }
    }
    else if (isPresent && !m_optional[i]) {
      BOOST_THROW_EXCEPTION(ArgumentError(REPO_PARAMETER_FIELD[i] + " is forbidden but present"));
    }
  }
}

RepoCommand::FieldValidator::FieldValidator()
  : m_required(REPO_PARAMETER_UBOUND)
  , m_optional(REPO_PARAMETER_UBOUND)
{
}

InsertCommand::InsertCommand()
: RepoCommand()
{
  m_requestValidator
    .required(REPO_PARAMETER_NAME)
    .required(REPO_PARAMETER_START_BLOCK_ID)
    .required(REPO_PARAMETER_END_BLOCK_ID);
}

InsertCheckCommand::InsertCheckCommand()
{
  m_requestValidator
    .required(REPO_PARAMETER_NAME)
    .required(REPO_PARAMETER_PROCESS_ID);
}

DeleteCommand::DeleteCommand()
{
  m_requestValidator
    .required(REPO_PARAMETER_NAME)
    .required(REPO_PARAMETER_START_BLOCK_ID)
    .required(REPO_PARAMETER_END_BLOCK_ID)
    .required(REPO_PARAMETER_PROCESS_ID);
}

void
DeleteCommand::check(const RepoCommandParameter& parameters) const
{
  if (parameters.hasStartBlockId() || parameters.hasEndBlockId()) {
    if (parameters.hasEndBlockId()) {
      SegmentNo startBlockId = parameters.getStartBlockId();
      SegmentNo endBlockId = parameters.getEndBlockId();

      if (startBlockId > endBlockId) {
        BOOST_THROW_EXCEPTION(ArgumentError("start block Id is bigger than end block Id"));
      }
    }
    else {
      BOOST_THROW_EXCEPTION(ArgumentError("Segmented deletion without EndBlockId, not implemented"));
    }
  }
}
} // namespace repo