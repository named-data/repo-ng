/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */
#include "read-handle.hpp"

namespace repo {

void
ReadHandle::onInterest(const Name& prefix, const Interest& interest)
{
  Data data;
  if (getStorageHandle().readData(interest, data)) {
    getFace().put(data);
  }
}

void
ReadHandle::onRegisterFailed(const Name& prefix, const std::string& reason)
{
  std::cerr << "ERROR: Failed to register prefix in local hub's daemon" << std::endl;
  getFace().shutdown();
}

void
ReadHandle::listen(const Name& prefix)
{
  getFace().setInterestFilter(prefix,
                              bind(&ReadHandle::onInterest, this, _1, _2),
                              bind(&ReadHandle::onRegisterFailed, this, _1, _2));
}

} //namespace repo
