/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */
#ifndef REPO_NDN_HANDLE_NDN_HANDLE_COMMON_HPP
#define REPO_NDN_HANDLE_NDN_HANDLE_COMMON_HPP


#include <cstdlib>
#include <sstream>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <ndn-cpp-dev/face.hpp>
#include <ndn-cpp-dev/security/key-chain.hpp>
#include "../storage/storage-handle.hpp"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <map>

#define RETRY_TIMEOUT 3
#define DEFAULT_CREDIT 12
#define NOEND_TIMEOUT 10000

namespace repo {
using ndn::Face;
using ndn::Name;
using ndn::Interest;
using ndn::KeyChain;
using ndn::Selectors;
using ndn::bind;
}

#endif
