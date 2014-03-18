/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "delete-handle.hpp"

namespace repo {

DeleteHandle::DeleteHandle(Face& face, StorageHandle& storageHandle, KeyChain& keyChain,
                           Scheduler& scheduler, CommandInterestValidator& validator)
  : BaseHandle(face, storageHandle, keyChain, scheduler)
  , m_validator(validator)
{
}

void
DeleteHandle::onInterest(const Name& prefix, const Interest& interest)
{
  //std::cout << "call DeleteHandle" << std::endl;
  m_validator.validate(interest, bind(&DeleteHandle::onValidated, this, _1, prefix),
                       bind(&DeleteHandle::onValidationFailed, this, _1, prefix));
}


void
DeleteHandle::onRegisterFailed(const Name& prefix, const std::string& reason)
{
  throw Error("Delete prefix registration failed");
}


void
DeleteHandle::onCheckInterest(const Name& prefix, const Interest& interest)
{
  BOOST_ASSERT(false); // Deletion progress check, not implemented
}


void
DeleteHandle::onCheckRegisterFailed(const Name& prefix, const std::string& reason)
{
  throw Error("Delete check prefix registration failed");
}


void
DeleteHandle::onValidated(const shared_ptr<const Interest>& interest, const Name& prefix)
{
  RepoCommandParameter parameter;

  try {
    extractParameter(*interest, prefix, parameter);
  }
  catch (RepoCommandParameter::Error) {
    negativeReply(*interest, 403);
    return;
  }

  if (parameter.hasSelectors()) {

    if (parameter.hasStartBlockId() || parameter.hasEndBlockId()) {
      negativeReply(*interest, 402);
      return;
    }

    //choose data with selector and delete it
    processSelectorDeleteCommand(*interest, parameter);
    return;
  }

  if (!parameter.hasStartBlockId() && !parameter.hasEndBlockId()) {
    processSingleDeleteCommand(*interest, parameter);
    return;
  }

  processSegmentDeleteCommand(*interest, parameter);
}

void
DeleteHandle::onValidationFailed(const shared_ptr<const Interest>& interest, const Name& prefix)
{
  std::cout << "invalidated" << std::endl;
  negativeReply(*interest, 401);
}

void
DeleteHandle::listen(const Name& prefix)
{
  getFace().setInterestFilter(prefix,
                              bind(&DeleteHandle::onInterest, this, _1, _2),
                              bind(&DeleteHandle::onRegisterFailed, this, _1, _2));
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
  uint64_t nDeletedDatas = 0;
  if (getStorageHandle().deleteData(parameter.getName())) {
    nDeletedDatas++;
  }
  positiveReply(interest, parameter, 200, nDeletedDatas);
}

void
DeleteHandle::processSelectorDeleteCommand(const Interest& interest,
                                           RepoCommandParameter& parameter)
{
  uint64_t nDeletedDatas = 0;
  Name name = parameter.getName();
  Selectors selectors = parameter.getSelectors();
  vector<Name> names;
  getStorageHandle().readNameAny(name, selectors, names);

  for (vector<Name>::iterator it = names.begin(); it != names.end(); ++it) {
    if (getStorageHandle().deleteData(*it)) {
      nDeletedDatas++;
    }
  }

  //All data has been deleted, return 200
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

} //namespace repo
