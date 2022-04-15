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

#include "delete-handle.hpp"

#include <ndn-cxx/util/logger.hpp>

namespace repo {

NDN_LOG_INIT(repo.DeleteHandle);

DeleteHandle::DeleteHandle(Face& face, RepoStorage& storageHandle,
                           ndn::mgmt::Dispatcher& dispatcher, Scheduler& scheduler,
                           ndn::security::Validator& validator)
  : CommandBaseHandle(face, storageHandle, scheduler, validator)
{
  dispatcher.addControlCommand<RepoCommandParameter>(ndn::PartialName("delete"),
    makeAuthorization(),
    std::bind(&DeleteHandle::validateParameters<DeleteCommand>, this, _1),
    std::bind(&DeleteHandle::handleDeleteCommand, this, _1, _2, _3, _4));

}

void
DeleteHandle::handleDeleteCommand(const Name& prefix, const Interest& interest,
                                  const ndn::mgmt::ControlParameters& parameter,
                                  const ndn::mgmt::CommandContinuation& done)
{
  const RepoCommandParameter& repoParameter = dynamic_cast<const RepoCommandParameter&>(parameter);

  if (!repoParameter.hasStartBlockId() && !repoParameter.hasEndBlockId()) {
    processSingleDeleteCommand(interest, repoParameter, done);
    return;
  }

  processSegmentDeleteCommand(interest, repoParameter, done);
}

RepoCommandResponse
DeleteHandle::positiveReply(const Interest& interest, const RepoCommandParameter& parameter,
                            uint64_t statusCode, uint64_t nDeletedData) const
{
  RepoCommandResponse response(statusCode, "Deletion Successful");

  if (parameter.hasProcessId()) {
    response.setProcessId(parameter.getProcessId());
    response.setDeleteNum(nDeletedData);
    response.setBody(response.wireEncode());
  }
  else {
    response.setCode(403);
    response.setText("Malformed Command");
    response.setBody(response.wireEncode());
  }
  return response;
}

RepoCommandResponse
DeleteHandle::negativeReply(const Interest& interest, uint64_t statusCode,
                            const std::string& text) const
{
  RepoCommandResponse response(statusCode, text);
  response.setBody(response.wireEncode());
  return response;
}

void
DeleteHandle::processSingleDeleteCommand(const Interest& interest, const RepoCommandParameter& parameter,
                                         const ndn::mgmt::CommandContinuation& done) const
{
  int64_t nDeletedData = storageHandle.deleteData(parameter.getName());
  if (nDeletedData == -1) {
    NDN_LOG_DEBUG("Deletion Failed");
    done(negativeReply(interest, 405, "Deletion Failed"));
  }
  else
  done(positiveReply(interest, parameter, 200, nDeletedData));
}

void
DeleteHandle::processSegmentDeleteCommand(const Interest& interest, const RepoCommandParameter& parameter,
                                          const ndn::mgmt::CommandContinuation& done) const
{
  SegmentNo startBlockId = parameter.hasStartBlockId() ? parameter.getStartBlockId() : 0;
  SegmentNo endBlockId = parameter.getEndBlockId();

  Name prefix = parameter.getName();
  uint64_t nDeletedData = 0;
  for (SegmentNo i = startBlockId; i <= endBlockId; i++) {
    Name name = prefix;
    name.appendSegment(i);
    if (storageHandle.deleteData(name)) {
      nDeletedData++;
    }
  }

  //All the data deleted, return 200
  done(positiveReply(interest, parameter, 200, nDeletedData));
}

} // namespace repo
