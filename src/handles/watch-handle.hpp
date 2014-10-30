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

#ifndef REPO_HANDLES_WATCH_HANDLE_HPP
#define REPO_HANDLES_WATCH_HANDLE_HPP

#include "base-handle.hpp"

#include <queue>

namespace repo {

using std::map;
using std::pair;
using std::queue;
using namespace ndn::time;
/**
 * @brief WatchHandle provides a different way for repo to insert data.
 *
 * Repo keeps sending interest to request the data with same prefix,
 *
 * but with different exclude selectors(updated every time). Repo will stop
 *
 * watching the prefix until a command interest tell it to stop, the total
 *
 * amount of sent interests reaches a specific number or time out.
 */
class WatchHandle : public BaseHandle
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
  WatchHandle(Face& face, RepoStorage& storageHandle, KeyChain& keyChain,
              Scheduler& scheduler, ValidatorConfig& validator);

  virtual void
  listen(const Name& prefix);

private: // watch-insert command
  /**
   * @brief handle watch commands
   */
  void
  onInterest(const Name& prefix, const Interest& interest);

  void
  onValidated(const std::shared_ptr<const Interest>& interest, const Name& prefix);

  void
  onValidationFailed(const std::shared_ptr<const Interest>& interest, const std::string& reason);

  void
  onRegistered(const Name& prefix);

  void
  onRegisterFailed(const Name& prefix, const std::string& reason);

private: // data fetching
  /**
   * @brief fetch data and send next interest
   */
  void
  onData(const Interest& interest, Data& data, const Name& name);

  /**
   * @brief handle when fetching one data timeout
   */
  void
  onTimeout(const Interest& interest, const Name& name);

  void
  onDataValidated(const Interest& interest, const std::shared_ptr<const Data>& data,
                  const Name& name);

  /**
   * @brief failure of validation
   */
  void
  onDataValidationFailed(const Interest& interest, const std::shared_ptr<const Data>& data,
                         const std::string& reason, const Name& name);


  void
  processWatchCommand(const Interest& interest, RepoCommandParameter& parameter);

  void
  watchStop(const Name& name);

private: // watch state check command
  /**
   * @brief handle watch check command
   */
  void
  onCheckInterest(const Name& prefix, const Interest& interest);

  void
  onCheckValidated(const std::shared_ptr<const Interest>& interest, const Name& prefix);

  void
  onCheckValidationFailed(const std::shared_ptr<const Interest>& interest,
                          const std::string& reason);

private: // watch stop command
  /**
   * @brief handle watch stop command
   */
  void
  onStopInterest(const Name& prefix, const Interest& interest);

  void
  onStopValidated(const std::shared_ptr<const Interest>& interest, const Name& prefix);

  void
  onStopValidationFailed(const std::shared_ptr<const Interest>& interest,
                         const std::string& reason);

private:
  void
  negativeReply(const Interest& interest, int statusCode);

  void
  deferredDeleteProcess(const Name& name);

  void
  deleteProcess(const Name& name);

  bool
  onRunning(const Name& name);

private:

  ValidatorConfig& m_validator;

  map<Name, std::pair<RepoCommandResponse, bool> > m_processes;
  int64_t m_interestNum;
  int64_t m_maxInterestNum;
  milliseconds m_interestLifetime;
  milliseconds m_watchTimeout;
  ndn::time::steady_clock::TimePoint m_startTime;
  int64_t m_size;
};

} // namespace repo

#endif // REPO_HANDLES_Watch_HANDLE_HPP
