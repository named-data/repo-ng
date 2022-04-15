/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018-2022, Regents of the University of California.
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
#include "extended-error-message.hpp"
#include "repo.hpp"

#include <iostream>
#include <string.h> // for strsignal()

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>

#include <ndn-cxx/util/logger.hpp>

NDN_LOG_INIT(repo.main);

int
main(int argc, char** argv)
{
  std::string configFile(DEFAULT_CONFIG_FILE);

  namespace po = boost::program_options;
  po::options_description optsDesc("Options");
  optsDesc.add_options()
    ("help,h",        "print this help message and exit")
    ("config-file,c", po::value<std::string>(&configFile)->default_value(configFile),
                      "path to configuration file")
    ;

  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, optsDesc), vm);
    po::notify(vm);
  }
  catch (const po::error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }
  catch (const boost::bad_any_cast& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 2;
  }

  if (vm.count("help") != 0) {
    std::cout << "Usage: " << argv[0] << " [options]\n"
              << "\n"
              << optsDesc;
    return 0;
  }

  boost::asio::io_service ioService;

  /// \todo reload config file on SIGHUP
  boost::asio::signal_set signalSet(ioService, SIGINT, SIGTERM);
  signalSet.async_wait([&ioService] (const boost::system::error_code& error, int signalNo) {
    if (!error) {
      NDN_LOG_FATAL("Exiting on signal " << signalNo << "/" << strsignal(signalNo));
      ioService.stop();
    }
  });

  try {
    repo::Repo repo(ioService, repo::parseConfig(configFile));
    repo.initializeStorage();
    repo.enableValidation();
    repo.enableListening();

    ioService.run();
  }
  catch (const std::exception& e) {
    NDN_LOG_FATAL(repo::getExtendedErrorMessage(e));
    return 1;
  }

  return 0;
}
