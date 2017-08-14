/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017, Regents of the University of California.
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

namespace repo {

DeleteHandle::DeleteHandle(Face& face, RepoStorage& storageHandle, KeyChain& keyChain,
                           Scheduler& scheduler,
                           Validator& validator)
  : BaseHandle(face, storageHandle, keyChain, scheduler)
  , m_validator(validator)
{
}

void
DeleteHandle::onInterest(const Name& prefix, const Interest& interest)
{
  m_validator.validate(interest, bind(&DeleteHandle::onValidated, this, _1, prefix),
                                 bind(&DeleteHandle::onValidationFailed, this, _1, _2));
}

void
DeleteHandle::onValidated(const Interest& interest, const Name& prefix)
{
  RepoCommandParameter parameter;

  try {
    extractParameter(interest, prefix, parameter);
  }
  catch (RepoCommandParameter::Error) {
    negativeReply(interest, 403);
    return;
  }

  if (parameter.hasSelectors()) {

    if (parameter.hasStartBlockId() || parameter.hasEndBlockId()) {
      negativeReply(interest, 402);
      return;
    }

    //choose data with selector and delete it
    processSelectorDeleteCommand(interest, parameter);
    return;
  }

  if (!parameter.hasStartBlockId() && !parameter.hasEndBlockId()) {
    processSingleDeleteCommand(interest, parameter);
    return;
  }

  processSegmentDeleteCommand(interest, parameter);
}

void
DeleteHandle::onValidationFailed(const Interest& interest, const ValidationError& error)
{
  std::cerr << error << std::endl;
  negativeReply(interest, 401);
}
//listen change the setinterestfilter
void
DeleteHandle::listen(const Name& prefix)
{
  getFace().setInterestFilter(Name(prefix).append("delete"),
                              bind(&DeleteHandle::onInterest, this, _1, _2));
}

void
DeleteHandle::positiveReply(const Interest& interest, const RepoCommandParameter& parameter,
                            uint64_t statusCode, uint64_t nDeletedDatas)
{
  RepoCommandResponse response;
  if (parameter.hasProcessId()) {
    response.setProcessId(parameter.getProcessId());
    response.setStatusCode(statusCode);
    response.setDeleteNum(nDeletedDatas);
  }
  else {
    response.setStatusCode(403);
  }
  reply(interest, response);
}

void
DeleteHandle::negativeReply(const Interest& interest, uint64_t statusCode)
{
  RepoCommandResponse response;
  response.setStatusCode(statusCode);
  reply(interest, response);
}

void
DeleteHandle::processSingleDeleteCommand(const Interest& interest,
                                         RepoCommandParameter& parameter)
{
  int64_t nDeletedDatas = getStorageHandle().deleteData(parameter.getName());
  if (nDeletedDatas == -1) {
    std::cerr << "Deletion Failed!" <<std::endl;
    negativeReply(interest, 405); //405 means deletion fail
  }
  else
    positiveReply(interest, parameter, 200, nDeletedDatas);
}

void
DeleteHandle::processSelectorDeleteCommand(const Interest& interest,
                                           RepoCommandParameter& parameter)
{
  int64_t nDeletedDatas = getStorageHandle()
                            .deleteData(Interest(parameter.getName())
                                          .setSelectors(parameter.getSelectors()));
  if (nDeletedDatas == -1) {
    std::cerr << "Deletion Failed!" <<std::endl;
    negativeReply(interest, 405); //405 means deletion fail
  }
  else
    positiveReply(interest, parameter, 200, nDeletedDatas);
}

void
DeleteHandle::processSegmentDeleteCommand(const Interest& interest,
                                          RepoCommandParameter& parameter)
{
  if (!parameter.hasStartBlockId())
    parameter.setStartBlockId(0);

  if (parameter.hasEndBlockId()) {
    SegmentNo startBlockId = parameter.getStartBlockId();
    SegmentNo endBlockId = parameter.getEndBlockId();

    if (startBlockId > endBlockId) {
      negativeReply(interest, 403);
      return;
    }

    Name prefix = parameter.getName();
    uint64_t nDeletedDatas = 0;
    for (SegmentNo i = startBlockId; i <= endBlockId; i++) {
      Name name = prefix;
      name.appendSegment(i);
      if (getStorageHandle().deleteData(name)) {
        nDeletedDatas++;
      }
    }
    //All the data deleted, return 200
    positiveReply(interest, parameter, 200, nDeletedDatas);
  }
  else {
    BOOST_ASSERT(false); // segmented deletion without EndBlockId, not implemented
  }
}

} // namespace repo
