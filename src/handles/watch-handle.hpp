/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef REPO_HANDLES_WATCH_HANDLE_HPP
#define REPO_HANDLES_WATCH_HANDLE_HPP

#include "command-base-handle.hpp"

#include <ndn-cxx/mgmt/dispatcher.hpp>

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
 * but with different exclude selectors(updated every time). Repo will stop
 * watching the prefix until a command interest tell it to stop, the total
 * amount of sent interests reaches a specific number or time out.
 */
class WatchHandle : public CommandBaseHandle
{

public:
  class Error : public CommandBaseHandle::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : CommandBaseHandle::Error(what)
    {
    }
  };


public:
  WatchHandle(Face& face, RepoStorage& storageHandle,
              ndn::mgmt::Dispatcher& dispatcher, Scheduler& scheduler,
              Validator& validator);

private: // watch-insert command
  /**
   * @brief handle watch commands
   */

  void
  handleStartCommand(const Name& prefix, const Interest& interest,
                     const ndn::mgmt::ControlParameters& parameters,
                     const ndn::mgmt::CommandContinuation& done);
  void
  onValidationFailed(const std::shared_ptr<const Interest>& interest, const std::string& reason);

private: // data fetching
  /**
   * @brief fetch data and send next interest
   */
  void
  onData(const Interest& interest, const Data& data, const Name& name);

  /**
   * @brief handle when fetching one data timeout
   */
  void
  onTimeout(const Interest& interest, const Name& name);

  void
  onDataValidated(const Interest& interest, const Data& data, const Name& name);

  /**
   * @brief failure of validation
   */
  void
  onDataValidationFailed(const Interest& interest, const Data& data,
                         const ValidationError& error, const Name& name);


  void
  processWatchCommand(const Interest& interest, const RepoCommandParameter& parameter,
                      const ndn::mgmt::CommandContinuation& done);

  void
  watchStop(const Name& name);

private: // watch state check command
  /**
   * @brief handle watch check command
   */

  void
  handleCheckCommand(const Name& prefix, const Interest& interest,
                     const ndn::mgmt::ControlParameters& parameters,
                     const ndn::mgmt::CommandContinuation& done);

  void
  onCheckValidationFailed(const Interest& interest, const ValidationError& error);

private: // watch stop command
  /**
   * @brief handle watch stop command
   */

  void
  handleStopCommand(const Name& prefix, const Interest& interest,
                    const ndn::mgmt::ControlParameters& parameters,
                    const ndn::mgmt::CommandContinuation& done);

  void
  onStopValidationFailed(const Interest& interest, const ValidationError& error);

private:
  void
  deferredDeleteProcess(const Name& name);

  void
  deleteProcess(const Name& name);

  bool
  onRunning(const Name& name);

private:
  Validator& m_validator;
  map<Name, std::pair<RepoCommandResponse, bool> > m_processes;
  int64_t m_interestNum;
  int64_t m_maxInterestNum;
  milliseconds m_interestLifetime;
  milliseconds m_watchTimeout;
  ndn::time::steady_clock::TimePoint m_startTime;
  int64_t m_size;
};

} // namespace repo

#endif // REPO_HANDLES_WATCH_HANDLE_HPP
