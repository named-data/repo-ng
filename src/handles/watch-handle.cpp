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

#include "watch-handle.hpp"

namespace repo {

static const milliseconds PROCESS_DELETE_TIME(10000_ms);
static const milliseconds DEFAULT_INTEREST_LIFETIME(4000_ms);

WatchHandle::WatchHandle(Face& face, RepoStorage& storageHandle,
                         ndn::mgmt::Dispatcher& dispatcher, Scheduler& scheduler, Validator& validator)
  : CommandBaseHandle(face, storageHandle, scheduler, validator)
  , m_validator(validator)
  , m_interestNum(0)
  , m_maxInterestNum(0)
  , m_interestLifetime(DEFAULT_INTEREST_LIFETIME)
  , m_watchTimeout(0)
  , m_startTime(steady_clock::now())
  , m_size(0)
{
  dispatcher.addControlCommand<RepoCommandParameter>(ndn::PartialName("watch").append("start"),
    makeAuthorization(),
    std::bind(&WatchHandle::validateParameters<WatchStartCommand>, this, _1),
    std::bind(&WatchHandle::handleStartCommand, this, _1, _2, _3, _4));

  dispatcher.addControlCommand<RepoCommandParameter>(ndn::PartialName("watch").append("check"),
    makeAuthorization(),
    std::bind(&WatchHandle::validateParameters<WatchCheckCommand>, this, _1),
    std::bind(&WatchHandle::handleCheckCommand, this, _1, _2, _3, _4));

  dispatcher.addControlCommand<RepoCommandParameter>(ndn::PartialName("watch").append("stop"),
    makeAuthorization(),
    std::bind(&WatchHandle::validateParameters<WatchStopCommand>, this, _1),
    std::bind(&WatchHandle::handleStopCommand, this, _1, _2, _3, _4));
}

void
WatchHandle::deleteProcess(const Name& name)
{
  m_processes.erase(name);
}

void
WatchHandle::handleStartCommand(const Name& prefix, const Interest& interest,
                                const ndn::mgmt::ControlParameters& parameter,
                                const ndn::mgmt::CommandContinuation& done)
{
  const RepoCommandParameter& repoParameter = dynamic_cast<const RepoCommandParameter&>(parameter);
  processWatchCommand(interest, repoParameter, done);
}

void WatchHandle::watchStop(const Name& name)
{
  m_processes[name].second = false;
  m_maxInterestNum = 0;
  m_interestNum = 0;
  m_startTime = steady_clock::now();
  m_watchTimeout = 0_ms;
  m_interestLifetime = DEFAULT_INTEREST_LIFETIME;
  m_size = 0;
}


void
WatchHandle::onData(const Interest& interest, const ndn::Data& data, const Name& name)
{
 m_validator.validate(data,
                      bind(&WatchHandle::onDataValidated, this, interest, _1, name),
                      bind(&WatchHandle::onDataValidationFailed, this, interest, _1, _2, name));
}

void
WatchHandle::onDataValidated(const Interest& interest, const Data& data, const Name& name)
{
  if (!m_processes[name].second) {
    return;
  }
  if (storageHandle.insertData(data)) {
    m_size++;
    if (!onRunning(name))
      return;

    Interest fetchInterest(interest.getName());
    fetchInterest.setSelectors(interest.getSelectors());
    fetchInterest.setInterestLifetime(m_interestLifetime);
    fetchInterest.setChildSelector(1);

    // update selectors
    // if data name is equal to interest name, use MinSuffixComponents selecor to exclude this data
    if (data.getName().size() == interest.getName().size()) {
      fetchInterest.setMinSuffixComponents(2);
    }
    else {
      Exclude exclude;
      if (!interest.getExclude().empty()) {
        exclude = interest.getExclude();
      }

      exclude.excludeBefore(data.getName()[interest.getName().size()]);
      fetchInterest.setExclude(exclude);
    }

    ++m_interestNum;
    face.expressInterest(fetchInterest,
                              bind(&WatchHandle::onData, this, _1, _2, name),
                              bind(&WatchHandle::onTimeout, this, _1, name), // Nack
                              bind(&WatchHandle::onTimeout, this, _1, name));
  }
  else {
    BOOST_THROW_EXCEPTION(Error("Insert into Repo Failed"));
  }
  m_processes[name].first.setInsertNum(m_size);
}

void
WatchHandle::onDataValidationFailed(const Interest& interest, const Data& data,
                                    const ValidationError& error, const Name& name)
{
  std::cerr << error << std::endl;
  if (!m_processes[name].second) {
    return;
  }
  if (!onRunning(name))
    return;

  Interest fetchInterest(interest.getName());
  fetchInterest.setSelectors(interest.getSelectors());
  fetchInterest.setInterestLifetime(m_interestLifetime);
  fetchInterest.setChildSelector(1);

  // update selectors
  // if data name is equal to interest name, use MinSuffixComponents selecor to exclude this data
  if (data.getName().size() == interest.getName().size()) {
    fetchInterest.setMinSuffixComponents(2);
  }
  else {
    Exclude exclude;
    if (!interest.getExclude().empty()) {
      exclude = interest.getExclude();
    }
    // Only exclude this data since other data whose names are smaller may be validated and satisfied
    exclude.excludeBefore(data.getName()[interest.getName().size()]);
    fetchInterest.setExclude(exclude);
  }

  ++m_interestNum;
  face.expressInterest(fetchInterest,
                            bind(&WatchHandle::onData, this, _1, _2, name),
                            bind(&WatchHandle::onTimeout, this, _1, name), // Nack
                            bind(&WatchHandle::onTimeout, this, _1, name));
}

void
WatchHandle::onTimeout(const ndn::Interest& interest, const Name& name)
{
  std::cerr << "Timeout" << std::endl;
  if (!m_processes[name].second) {
    return;
  }
  if (!onRunning(name))
    return;
  // selectors do not need to be updated
  Interest fetchInterest(interest.getName());
  fetchInterest.setSelectors(interest.getSelectors());
  fetchInterest.setInterestLifetime(m_interestLifetime);
  fetchInterest.setChildSelector(1);

  ++m_interestNum;
  face.expressInterest(fetchInterest,
                            bind(&WatchHandle::onData, this, _1, _2, name),
                            bind(&WatchHandle::onTimeout, this, _1, name), // Nack
                            bind(&WatchHandle::onTimeout, this, _1, name));

}

void
WatchHandle::handleStopCommand(const Name& prefix, const Interest& interest,
                               const ndn::mgmt::ControlParameters& parameter,
                               const ndn::mgmt::CommandContinuation& done)
{
  const RepoCommandParameter& repoParameter = dynamic_cast<const RepoCommandParameter&>(parameter);

  watchStop(repoParameter.getName());
  std::string text = "Watched Prefix Insertion for prefix (" + prefix.toUri() + ") is stop.";
  return done(RepoCommandResponse(101, text));
}

void
WatchHandle::handleCheckCommand(const Name& prefix, const Interest& interest,
                                const ndn::mgmt::ControlParameters& parameter,
                                const ndn::mgmt::CommandContinuation& done)
{
  const RepoCommandParameter& repoParameter = dynamic_cast<const RepoCommandParameter&>(parameter);

  //check whether this process exists
  Name name = repoParameter.getName();
  if (m_processes.count(name) == 0) {
    std::cerr << "no such process name: " << name << std::endl;
    RepoCommandResponse response(404, "No such process is in progress");
    response.setBody(response.wireEncode());
    return done(response);
  }

  RepoCommandResponse& response = m_processes[name].first;

  if (!m_processes[name].second) {
    response.setCode(101);
  }

  return done(response);
}

void
WatchHandle::deferredDeleteProcess(const Name& name)
{
  scheduler.scheduleEvent(PROCESS_DELETE_TIME,
                               bind(&WatchHandle::deleteProcess, this, name));
}

void
WatchHandle::processWatchCommand(const Interest& interest,
                                 const RepoCommandParameter& parameter,
                                 const ndn::mgmt::CommandContinuation& done)
{
  // if there is no watchTimeout specified, m_watchTimeout will be set as 0 and this handle will run forever
  if (parameter.hasWatchTimeout()) {
    m_watchTimeout = parameter.getWatchTimeout();
  }
  else {
    m_watchTimeout = 0_ms;
  }

  // if there is no maxInterestNum specified, m_maxInterestNum will be 0, which means infinity
  if (parameter.hasMaxInterestNum()) {
    m_maxInterestNum = parameter.getMaxInterestNum();
  }
  else {
    m_maxInterestNum = 0;
  }

  if (parameter.hasInterestLifetime()) {
    m_interestLifetime = parameter.getInterestLifetime();
  }

  RepoCommandResponse response(100, "Watching the prefix started.");
  response.setBody(response.wireEncode());
  done(response);

  m_processes[parameter.getName()] =
                std::make_pair(RepoCommandResponse(300, "This watched prefix Insertion is in progress"),
                               true);

  Interest fetchInterest(parameter.getName());
  if (parameter.hasSelectors()) {
    fetchInterest.setSelectors(parameter.getSelectors());
  }
  fetchInterest.setChildSelector(1);
  fetchInterest.setInterestLifetime(m_interestLifetime);
  m_startTime = steady_clock::now();
  m_interestNum++;
  face.expressInterest(fetchInterest,
                       bind(&WatchHandle::onData, this, _1, _2, parameter.getName()),
                       bind(&WatchHandle::onTimeout, this, _1, parameter.getName()), // Nack
                       bind(&WatchHandle::onTimeout, this, _1, parameter.getName()));
}

bool
WatchHandle::onRunning(const Name& name)
{
  bool isTimeout = (m_watchTimeout != milliseconds::zero() &&
                    steady_clock::now() - m_startTime > m_watchTimeout);
  bool isMaxInterest = m_interestNum >= m_maxInterestNum && m_maxInterestNum != 0;
  if (isTimeout || isMaxInterest) {
    deferredDeleteProcess(name);
    watchStop(name);
    return false;
  }
  return true;
}

} // namespace repo
