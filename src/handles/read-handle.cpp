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

#include "read-handle.hpp"

namespace repo {

void
ReadHandle::onInterest(const Name& prefix, const Interest& interest)
{

  shared_ptr<ndn::Data> data = getStorageHandle().readData(interest);
  if (data != NULL) {
      getFace().put(*data);
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
  ndn::InterestFilter filter(prefix);
  getFace().setInterestFilter(filter,
                              bind(&ReadHandle::onInterest, this, _1, _2),
                              bind(&ReadHandle::onRegisterFailed, this, _1, _2));
}

} //namespace repo
