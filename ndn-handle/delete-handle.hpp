/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_NDN_HANDLE_DELETE_HANDLE_HPP
#define REPO_NDN_HANDLE_DELETE_HANDLE_HPP

#include "ndn-handle-common.hpp"
#include "base-handle.hpp"

namespace repo {

using std::vector;

class DeleteHandle : public BaseHandle
{

public:
  class Error : public BaseHandle::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : BaseHandle::Error(what)
    {
    }
  };

public:
  DeleteHandle(Face& face, StorageHandle& storageHandle, KeyChain& keyChain,
               Scheduler& scheduler, CommandInterestValidator& validator);

  virtual void
  listen(const Name& prefix);

private:
  void
  onInterest(const Name& prefix, const Interest& interest);

  void
  onRegisterFailed(const Name& prefix, const std::string& reason);

  void
  onValidated(const shared_ptr<const Interest>& interest, const Name& prefix);

  void
  onValidationFailed(const shared_ptr<const Interest>& interest, const Name& prefix);

  /**
  * @todo delete check has not been realized due to the while loop of segmented data deletion.
  */
  void
  onCheckInterest(const Name& prefix, const Interest& interest);

  void
  onCheckRegisterFailed(const Name& prefix, const std::string& reason);

  void
  positiveReply(const Interest& interest, const RepoCommandParameter& parameter,
                uint64_t statusCode, uint64_t nDeletedDatas);

  void
  negativeReply(const Interest& interest, uint64_t statusCode);

  void
  processSingleDeleteCommand(const Interest& interest, RepoCommandParameter& parameter);

  void
  processSelectorDeleteCommand(const Interest& interest, RepoCommandParameter& parameter);

  void
  processSegmentDeleteCommand(const Interest& interest, RepoCommandParameter& parameter);

private:
  CommandInterestValidator& m_validator;

};

} //namespace repo

#endif // REPO_NDN_HANDLE_DELETE_HANDLE_HPP
