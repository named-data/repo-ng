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

#ifndef REPO_HANDLES_DELETE_HANDLE_HPP
#define REPO_HANDLES_DELETE_HANDLE_HPP

#include "base-handle.hpp"
#include <ndn-cxx/security/validator-config.hpp>

namespace repo {

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
  DeleteHandle(Face& face, RepoStorage& storageHandle, KeyChain& keyChain,
               Scheduler& scheduler, ValidatorConfig& validator);

  virtual void
  listen(const Name& prefix);

private:
  void
  onInterest(const Name& prefix, const Interest& interest);

  void
  onRegisterFailed(const Name& prefix, const std::string& reason);

  void
  onValidated(const std::shared_ptr<const Interest>& interest, const Name& prefix);

  void
  onValidationFailed(const std::shared_ptr<const Interest>& interest, const std::string& reason);

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
  ValidatorConfig& m_validator;

};

} // namespace repo

#endif // REPO_HANDLES_DELETE_HANDLE_HPP
