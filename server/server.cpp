/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
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

    repoInstance.enableListening();

    ioService.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }

  return 0;
}
