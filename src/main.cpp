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

#include "config.hpp"
#include "repo.hpp"

using namespace repo;

static const string ndnRepoUsageMessage =
  /* argv[0] */ " - Next generation of NDN repository\n"
  "-h: show help message\n"
  "-c: set config file path\n"
  ;

void
terminate(boost::asio::io_service& ioService,
          const boost::system::error_code& error,
          int signalNo,
          boost::asio::signal_set& signalSet)
{
  if (error)
    return;

  if (signalNo == SIGINT ||
      signalNo == SIGTERM)
    {
      ioService.stop();
      std::cout << "Caught signal '" << strsignal(signalNo) << "', exiting..." << std::endl;
    }
  else
    {
      /// \todo May be try to reload config file
      signalSet.async_wait(bind(&terminate, boost::ref(ioService), _1, _2,
                                boost::ref(signalSet)));
    }
}

int
main(int argc, char** argv)
{
  string configPath = DEFAULT_CONFIG_FILE;
  int opt;
  while ((opt = getopt(argc, argv, "hc:")) != -1) {
    switch (opt) {
    case 'h':
      std::cout << argv[0] << ndnRepoUsageMessage << std::endl;
      return 1;
    case 'c':
      configPath = string(optarg);
      break;
    default:
      break;
    }
  }

  try {
    boost::asio::io_service ioService;
    Repo repoInstance(ioService, parseConfig(configPath));

    boost::asio::signal_set signalSet(ioService);
    signalSet.add(SIGINT);
    signalSet.add(SIGTERM);
    signalSet.add(SIGHUP);
    signalSet.add(SIGUSR1);
    signalSet.add(SIGUSR2);
    signalSet.async_wait(bind(&terminate, boost::ref(ioService), _1, _2,
                              boost::ref(signalSet)));

    repoInstance.initializeStorage();

    repoInstance.enableValidation();

    repoInstance.enableListening();

    ioService.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }

  return 0;
}
