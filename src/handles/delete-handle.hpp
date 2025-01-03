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

#ifndef REPO_HANDLES_DELETE_HANDLE_HPP
#define REPO_HANDLES_DELETE_HANDLE_HPP

#include "command-base-handle.hpp"

namespace repo {

class DeleteHandle : public CommandBaseHandle
{
public:
  class Error : public CommandBaseHandle::Error
  {
  public:
    using CommandBaseHandle::Error::Error;
  };

  DeleteHandle(Face& face, RepoStorage& storageHandle,
               ndn::mgmt::Dispatcher& dispatcher, Scheduler& scheduler,
               ndn::security::Validator& validator);

private:
  void
  handleDeleteCommand(const Name& prefix, const Interest& interest,
                      const ndn::mgmt::ControlParametersBase& parameters,
                      const ndn::mgmt::CommandContinuation& done);

  RepoCommandResponse
  positiveReply(const Interest& interest, const RepoCommandParameter& parameter,
                uint64_t statusCode, uint64_t nDeletedData) const;

  RepoCommandResponse
  negativeReply(const Interest& interest, uint64_t statusCode, const std::string& text) const;

  void
  processSingleDeleteCommand(const Interest& interest, const RepoCommandParameter& parameter,
                             const ndn::mgmt::CommandContinuation& done) const;

  void
  processSegmentDeleteCommand(const Interest& interest, const RepoCommandParameter& parameter,
                              const ndn::mgmt::CommandContinuation& done) const;
};

} // namespace repo

#endif // REPO_HANDLES_DELETE_HANDLE_HPP
