/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_HELPERS_REPO_TLV_HPP
#define REPO_HELPERS_REPO_TLV_HPP

#include <ndn-cpp-dev/encoding/tlv.hpp>

namespace repo {
namespace tlv {

using namespace ndn::Tlv;

enum {
  RepoCommandParameter = 201,
  StartBlockId         = 204,
  EndBlockId           = 205,
  ProcessId            = 206,
  RepoCommandResponse  = 207,
  StatusCode           = 208,
  InsertNum            = 209,
  DeleteNum            = 210
};

} // tlv
} // repo

#endif // REPO_HELPERS_REPO_TLV_HPP
