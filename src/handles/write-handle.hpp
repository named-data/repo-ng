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

#ifndef REPO_HANDLES_WRITE_HANDLE_HPP
#define REPO_HANDLES_WRITE_HANDLE_HPP

#include "command-base-handle.hpp"

#include <ndn-cxx/util/segment-fetcher.hpp>

#include <queue>

namespace repo {

/**
 * @brief WriteHandle provides basic credit based congestion control.
 *
 * First repo sends interests of credit number and then credit will be 0.
 *
 * If a data comes, credit++ and sends a interest then credit--.
 *
 * If the interest timeout, repo will retry and send interest in retrytimes.
 *
 * If one interest timeout beyond retrytimes, the fetching process will terminate.
 *
 * Another case is that if command will insert segmented data without EndBlockId.
 *
 * The repo will keep fetching data in noendTimeout time.
 *
 * If data returns with FinalBlockId, this detecting timeout process will terminate.
 *
 * If client sends a insert check command, the noendTimeout timer will be set to 0.
 *
 * If repo cannot get FinalBlockId in noendTimeout time, the fetching process will terminate.
 */
class WriteHandle : public CommandBaseHandle
{
public:
  class Error : public CommandBaseHandle::Error
  {
  public:
    using CommandBaseHandle::Error::Error;
  };

  WriteHandle(Face& face, RepoStorage& storageHandle,
              ndn::mgmt::Dispatcher& dispatcher, Scheduler& scheduler,
              ndn::security::Validator& validator);

private:
  /**
  * @brief Information of insert process including variables for response
  *        and credit based flow control
  */
  struct ProcessInfo
  {
    RepoCommandResponse response;
    std::queue<SegmentNo> nextSegmentQueue;  ///< queue of waiting segment
                                        ///  to be sent when having credits
    SegmentNo nextSegment;  ///< last segment put into the nextSegmentQueue
    std::map<SegmentNo, int> retryCounts;  ///< to store retrying times of timeout segment
    int credit;  ///< congestion control credits of process

    /**
     * @brief the latest time point at which EndBlockId must be determined
     *
     * Segmented fetch process will terminate if EndBlockId cannot be
     * determined before this time point.
     * It is initialized to now()+noEndTimeout when segmented fetch process begins,
     * and reset to now()+noEndTimeout each time an insert status check command is processed.
     */
    time::steady_clock::time_point noEndTime;
  };

private: // insert command
  /**
   * @brief handle insert commands
   */
  void
  handleInsertCommand(const Name& prefix, const Interest& interest,
                      const ndn::mgmt::ControlParameters& parameters,
                      const ndn::mgmt::CommandContinuation& done);

  void
  onValidationFailed(const Interest& interest, const ndn::security::ValidationError& error);

private: // single data fetching
  /**
   * @brief fetch one data
   */
  void
  onData(const Interest& interest, const Data& data, ProcessId processId);

  void
  onDataValidated(const Interest& interest, const Data& data, ProcessId processId);

  /**
   * @brief handle when fetching one data timeout
   */
  void
  onTimeout(const Interest& interest, ProcessId processId);

  void
  processSingleInsertCommand(const Interest& interest, RepoCommandParameter& parameter,
                             const ndn::mgmt::CommandContinuation& done);

private:  // segmented data fetching
  /**
   * @brief fetch segmented data
   */
  void
  onSegmentData(ndn::util::SegmentFetcher& fetcher, const Data& data, ProcessId processId);

  /**
   * @brief handle when fetching segmented data timeout
   */
  void
  onSegmentTimeout(ndn::util::SegmentFetcher& fetcher, ProcessId processId);

  /**
   * @brief initiate fetching segmented data
   */
  void
  segInit(ProcessId processId, const RepoCommandParameter& parameter);

  void
  processSegmentedInsertCommand(const Interest& interest, RepoCommandParameter& parameter,
                                const ndn::mgmt::CommandContinuation& done);

private:
  /**
   * @brief extends noEndTime of process if not noEndTimeout, set StatusCode 405
   *
   * called by onCheckValidated() if there is no EndBlockId. If not noEndTimeout,
   * extends noEndTime of process. If noEndTimeout, set status code 405 as noEndTimeout.
   */
  void
  extendNoEndTime(ProcessInfo& process);

private: // insert state check command
  /**
   * @brief handle insert check command
   */

  void
  handleCheckCommand(const Name& prefix, const Interest& interest,
                     const ndn::mgmt::ControlParameters& parameters,
                     const ndn::mgmt::CommandContinuation& done);

  void
  onCheckValidationFailed(const Interest& interest, const ndn::security::ValidationError& error);

private:
  void
  deleteProcess(ProcessId processId);

  /**
   * @brief schedule a event to delete the process
   */
  void
  deferredDeleteProcess(ProcessId processId);

  RepoCommandResponse
  negativeReply(std::string text, int statusCode);

private:
  ndn::security::Validator& m_validator;
  std::map<ProcessId, ProcessInfo> m_processes;
  time::milliseconds m_interestLifetime = ndn::DEFAULT_INTEREST_LIFETIME;
};

} // namespace repo

#endif // REPO_HANDLES_WRITE_HANDLE_HPP
