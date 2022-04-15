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

#ifndef REPO_COMMON_HPP
#define REPO_COMMON_HPP

#include "config.hpp"

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/time.hpp>

#include <boost/core/noncopyable.hpp>

#include <algorithm>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef HAVE_TESTS
#define PUBLIC_WITH_TESTS_ELSE_PRIVATE public
#else
#define PUBLIC_WITH_TESTS_ELSE_PRIVATE private
#endif

namespace repo {

using boost::noncopyable;

using ndn::Face;
using ndn::Block;
using ndn::Name;
using ndn::Interest;
using ndn::Data;
using ndn::Scheduler;

using ndn::operator""_block;
namespace time = ndn::time;
using namespace ndn::time_literals;

using ProcessId = uint64_t;
using SegmentNo = uint64_t;

} // namespace repo

#endif // REPO_COMMON_HPP
