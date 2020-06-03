/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020, Regents of the University of California.
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

#include "command-base-handle.hpp"

#include <ndn-cxx/util/random.hpp>

namespace repo {

/** \brief an Interest tag to indicate command signer
 */
using SignerTag = ndn::SimpleTag<ndn::Name, 20>;

/** \brief obtain signer from SignerTag attached to Interest, if available
 */
static ndn::optional<std::string>
getSignerFromTag(const ndn::Interest& interest)
{
  std::shared_ptr<SignerTag> signerTag = interest.getTag<SignerTag>();
  if (signerTag == nullptr) {
    return ndn::nullopt;
  }
  else {
    return signerTag->get().toUri();
  }
}

CommandBaseHandle::CommandBaseHandle(Face& face, RepoStorage& storageHandle,
                  Scheduler& scheduler, Validator& validator)
  : face(face)
  , storageHandle(storageHandle)
  , scheduler(scheduler)
  , m_validator(validator)
{
}

ndn::mgmt::Authorization
CommandBaseHandle::makeAuthorization()
{
  return [=] (const ndn::Name& prefix, const ndn::Interest& interest,
            const ndn::mgmt::ControlParameters* params,
            const ndn::mgmt::AcceptContinuation& accept,
            const ndn::mgmt::RejectContinuation& reject) {
  m_validator.validate(interest,
    [accept] (const ndn::Interest& request) {

      auto signer1 = getSignerFromTag(request);
      std::string signer = signer1.value_or("*");
      accept(signer);
    },
    [reject] (const ndn::Interest& request,
              const ndn::security::ValidationError& error) {
      reject(ndn::mgmt::RejectReply::STATUS403);
    });
  };
}

} // namespace repo
