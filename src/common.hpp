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

#ifndef REPO_COMMON_HPP
#define REPO_COMMON_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/selectors.hpp>
#include <ndn-cxx/key-locator.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include <boost/utility.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/geometric_distribution.hpp>

#include <map>
#include <string>
#include <vector>
#include <queue>
#include <list>
#include <algorithm>
#include <iostream>

namespace repo {

using ndn::Face;
using ndn::Block;
using ndn::Name;
using ndn::Interest;
using ndn::Selectors;
using ndn::Exclude;
using ndn::Data;
using ndn::KeyLocator;
using ndn::KeyChain;
using ndn::Scheduler;
using ndn::ValidatorConfig;

using ndn::bind;
using ndn::shared_ptr;
using ndn::make_shared;
using ndn::enable_shared_from_this;

using std::vector;
using std::string;

using boost::noncopyable;

typedef uint64_t ProcessId;
typedef uint64_t SegmentNo;

} // namespace repo

#endif // REPO_COMMON_HPP
