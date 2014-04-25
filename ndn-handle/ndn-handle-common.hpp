/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
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
