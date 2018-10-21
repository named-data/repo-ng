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

#include "../src/repo-command-parameter.hpp"
#include "../src/repo-command-response.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/command-interest-signer.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>

#include <boost/asio/io_service.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include <stdint.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/lexical_cast.hpp>

namespace repo {

using namespace ndn::time;

using std::shared_ptr;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;

static const uint64_t DEFAULT_INTEREST_LIFETIME = 4000;
static const uint64_t DEFAULT_FRESHNESS_PERIOD = 10000;
static const uint64_t DEFAULT_CHECK_PERIOD = 1000;

enum CommandType
{
  START,
  CHECK,
  STOP
};

class NdnRepoWatch : boost::noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  NdnRepoWatch()
    : freshnessPeriod(DEFAULT_FRESHNESS_PERIOD)
    , interestLifetime(DEFAULT_INTEREST_LIFETIME)
    , hasTimeout(false)
    , watchTimeout(0)
    , hasMaxInterestNum(false)
    , maxInterestNum(0)
    , status(START)
    , isVerbose(false)
    , m_scheduler(m_face.getIoService())
    , m_checkPeriod(DEFAULT_CHECK_PERIOD)
    , m_cmdSigner(m_keyChain)
  {
  }

  void
  run();

private:

  void
  startWatchCommand();

  void
  onWatchCommandResponse(const ndn::Interest& interest, const ndn::Data& data);

  void
  onWatchCommandTimeout(const ndn::Interest& interest);

  void
  stopProcess();

  void
  signData(ndn::Data& data);

  void
  startCheckCommand();

  void
  onCheckCommandTimeout(const ndn::Interest& interest);

  void
  onStopCommandResponse(const ndn::Interest& interest, ndn::Data& data);

  void
  onStopCommandTimeout(const ndn::Interest& interest);

  ndn::Interest
  generateCommandInterest(const ndn::Name& commandPrefix, const std::string& command,
                          const RepoCommandParameter& commandParameter);

public:
  std::string identityForCommand;
  milliseconds freshnessPeriod;
  milliseconds interestLifetime;
  bool hasTimeout;
  milliseconds watchTimeout;
  bool hasMaxInterestNum;
  int64_t maxInterestNum;
  CommandType status;
  ndn::Name repoPrefix;
  ndn::Name ndnName;
  bool isVerbose;


private:
  ndn::Face m_face;
  ndn::Scheduler m_scheduler;
  milliseconds m_checkPeriod;

  ndn::Name m_dataPrefix;
  ndn::KeyChain m_keyChain;
  typedef std::map<uint64_t, shared_ptr<ndn::Data> > DataContainer;
  ndn::security::CommandInterestSigner m_cmdSigner;
};

void
NdnRepoWatch::run()
{
  m_dataPrefix = ndnName;
  startWatchCommand();

  if (hasTimeout)
    m_scheduler.scheduleEvent(watchTimeout, bind(&NdnRepoWatch::stopProcess, this));

  m_face.processEvents();
}

void
NdnRepoWatch::startWatchCommand()
{
  RepoCommandParameter parameters;
  parameters.setName(m_dataPrefix);

  repoPrefix.append("watch");
  if (status == START) {
    if (hasMaxInterestNum) {
      parameters.setMaxInterestNum(maxInterestNum);
    }
    if (hasTimeout) {
      parameters.setWatchTimeout(watchTimeout);
    }
    ndn::Interest commandInterest = generateCommandInterest(repoPrefix, "start", parameters);
    m_face.expressInterest(commandInterest,
                           bind(&NdnRepoWatch::onWatchCommandResponse, this,
                                     _1, _2),
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1), // Nack
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1));
  }
  else if (status == STOP){
    ndn::Interest commandInterest = generateCommandInterest(repoPrefix, "stop", parameters);
    m_face.expressInterest(commandInterest,
                           bind(&NdnRepoWatch::onWatchCommandResponse, this,
                                     _1, _2),
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1), // Nack
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1));
  }
  else if (status == CHECK){
    ndn::Interest commandInterest = generateCommandInterest(repoPrefix, "check", parameters);
    m_face.expressInterest(commandInterest,
                           bind(&NdnRepoWatch::onWatchCommandResponse, this,
                                     _1, _2),
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1), // Nack
                           bind(&NdnRepoWatch::onWatchCommandTimeout, this, _1));
  }

}

void
NdnRepoWatch::onWatchCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
  RepoCommandResponse response(data.getContent().blockFromValue());
  int statusCode = response.getCode();
  if (statusCode >= 400) {
    BOOST_THROW_EXCEPTION(Error("Watch command failed with code " +
                                boost::lexical_cast<std::string>(statusCode)));
  }
  else if (statusCode == 101) {
    std::cerr << "Watching prefix is stopped!" <<std::endl;
    m_face.getIoService().stop();
    return;
  }
  else if (statusCode == 300) {
    std::cerr << "Watching prefix is running!" <<std::endl;
    m_scheduler.scheduleEvent(m_checkPeriod,
                              bind(&NdnRepoWatch::startCheckCommand, this));
    return;
  }
  else if (statusCode == 100) {
    std::cerr << "Watching prefix starts!" <<std::endl;
    m_scheduler.scheduleEvent(m_checkPeriod,
                              bind(&NdnRepoWatch::startCheckCommand, this));
    return;
  }
  else {
    BOOST_THROW_EXCEPTION(Error("Unrecognized Status Code " +
                                boost::lexical_cast<std::string>(statusCode)));
  }
}

void
NdnRepoWatch::onWatchCommandTimeout(const ndn::Interest& interest)
{
  BOOST_THROW_EXCEPTION(Error("command response timeout"));
}

void
NdnRepoWatch::stopProcess()
{
  m_face.getIoService().stop();
}

void
NdnRepoWatch::startCheckCommand()
{
  repoPrefix.append("watch");
  ndn::Interest checkInterest = generateCommandInterest(repoPrefix, "check",
                                                        RepoCommandParameter()
                                                          .setName(m_dataPrefix));
  m_face.expressInterest(checkInterest,
                         bind(&NdnRepoWatch::onWatchCommandResponse, this, _1, _2),
                         bind(&NdnRepoWatch::onCheckCommandTimeout, this, _1), // Nack
                         bind(&NdnRepoWatch::onCheckCommandTimeout, this, _1));
}

void
NdnRepoWatch::onCheckCommandTimeout(const ndn::Interest& interest)
{
  BOOST_THROW_EXCEPTION(Error("check response timeout"));
}

void
NdnRepoWatch::onStopCommandResponse(const ndn::Interest& interest, ndn::Data& data)
{
  RepoCommandResponse response(data.getContent().blockFromValue());
  int statusCode = response.getCode();
  if (statusCode != 101) {
    BOOST_THROW_EXCEPTION(Error("Watch stop command failed with code: " +
                                boost::lexical_cast<std::string>(statusCode)));
  }
  else {
    std::cerr << "Status code is 101. Watching prefix is stopped successfully!" << std::endl;
    m_face.getIoService().stop();
    return;
  }
}

void
NdnRepoWatch::onStopCommandTimeout(const ndn::Interest& interest)
{
  BOOST_THROW_EXCEPTION(Error("stop response timeout"));
}

ndn::Interest
NdnRepoWatch::generateCommandInterest(const ndn::Name& commandPrefix, const std::string& command,
                                      const RepoCommandParameter& commandParameter)
{
  Name cmd = commandPrefix;
  cmd
    .append(command)
    .append(commandParameter.wireEncode());
  ndn::Interest interest;

  if (identityForCommand.empty())
    interest = m_cmdSigner.makeCommandInterest(cmd);
  else {
    interest = m_cmdSigner.makeCommandInterest(cmd, ndn::signingByIdentity(identityForCommand));
  }

  interest.setInterestLifetime(interestLifetime);
  return interest;
}

static void
usage()
{
  fprintf(stderr,
          "NdnRepoWatch [-I identity]"
          "  [-x freshness] [-l lifetime] [-w watchtimeout]"
          "  [-n maxinterestnum][-s stop] [-c check]repo-prefix ndn-name\n"
          "\n"
          " Write a file into a repo.\n"
          "  -I: specify identity used for signing commands\n"
          "  -x: FreshnessPeriod in milliseconds\n"
          "  -l: InterestLifetime in milliseconds for each command\n"
          "  -w: timeout in milliseconds for whole process (default unlimited)\n"
          "  -n: total number of interests to be sent for whole process (default unlimited)\n"
          "  -s: stop the whole process\n"
          "  -c: check the process\n"
          "  repo-prefix: repo command prefix\n"
          "  ndn-name: NDN Name prefix for written Data\n"
          );
  exit(1);
}

int
main(int argc, char** argv)
{
  NdnRepoWatch app;
  int opt;
  while ((opt = getopt(argc, argv, "x:l:w:n:scI:")) != -1) {
    switch (opt) {
    case 'x':
      try {
        app.freshnessPeriod = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-x option should be an integer.";
        return 1;
      }
      break;
    case 'l':
      try {
        app.interestLifetime = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-l option should be an integer.";
        return 1;
      }
      break;
    case 'w':
      app.hasTimeout = true;
      try {
        app.watchTimeout = milliseconds(boost::lexical_cast<int64_t>(optarg));
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-w option should be an integer.";
        return 1;
      }
      break;
    case 'n':
      app.hasMaxInterestNum = true;
      try {
        app.maxInterestNum = boost::lexical_cast<int64_t>(optarg);
      }
      catch (const boost::bad_lexical_cast&) {
        std::cerr << "-n option should be an integer.";
        return 1;
      }
      break;
    case 's':
      app.status = STOP;
      break;
    case 'c':
      app.status = CHECK;
      break;
    case 'I':
      app.identityForCommand = std::string(optarg);
      break;
    case 'v':
      app.isVerbose = true;
      break;
    case 'h':
      usage();
      break;
    default:
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 2)
    usage();

  app.repoPrefix = Name(argv[0]);
  app.ndnName = Name(argv[1]);

  app.run();

  return 0;
}

} // namespace repo

int
main(int argc, char** argv)
{
  try {
    return repo::main(argc, argv);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
}
