/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2013 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "base-handle.hpp"

namespace repo {

uint64_t
BaseHandle::generateProcessId()
{
  static boost::random::mt19937_64 gen;
  static boost::random::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFFLL);
  return dist(gen);
}

}