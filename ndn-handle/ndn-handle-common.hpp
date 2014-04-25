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

#ifndef REPO_NDN_HANDLE_NDN_HANDLE_COMMON_HPP
#define REPO_NDN_HANDLE_NDN_HANDLE_COMMON_HPP


#include "../storage/storage-handle.hpp"
#include "../helpers/repo-command-response.hpp"
#include "../helpers/repo-command-parameter.hpp"

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/command-interest-validator.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <map>
#include <algorithm>

namespace repo {

using ndn::Face;
using ndn::Name;
using ndn::Interest;
using ndn::KeyChain;
using ndn::Selectors;
using ndn::bind;
using ndn::CommandInterestValidator;
using ndn::Scheduler;

using ndn::shared_ptr;
using ndn::make_shared;
using ndn::enable_shared_from_this;

typedef uint64_t ProcessId;
typedef uint64_t SegmentNo;

}

#endif // REPO_NDN_HANDLE_NDN_HANDLE_COMMON_HPP
