/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025, Regents of the University of California.
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

#include <optional>

namespace repo {

/** \brief an Interest tag to indicate command signer
 */
using SignerTag = ndn::SimpleTag<ndn::Name, 20>;

/** \brief obtain signer from SignerTag attached to Interest, if available
 */
static std::optional<std::string>
getSignerFromTag(const ndn::Interest& interest)
{
  auto signerTag = interest.getTag<SignerTag>();
  if (signerTag == nullptr) {
    return std::nullopt;
  }
  else {
    return signerTag->get().toUri();
  }
}

CommandBaseHandle::CommandBaseHandle(Face& face, RepoStorage& storage,
                                     Scheduler& sched, ndn::security::Validator& validator)
  : face(face)
  , storageHandle(storage)
  , scheduler(sched)
  , m_validator(validator)
{
}

ndn::mgmt::Authorization
CommandBaseHandle::makeAuthorization()
{
  return [=] (const ndn::Name&, const auto& interest,
              const ndn::mgmt::ControlParametersBase*,
              const ndn::mgmt::AcceptContinuation& accept,
              const ndn::mgmt::RejectContinuation& reject) {
  m_validator.validate(interest,
    [accept] (const auto& request) {
      auto signer = getSignerFromTag(request).value_or("*");
      accept(signer);
    },
    [reject] (auto&&...) {
      reject(ndn::mgmt::RejectReply::STATUS403);
    });
  };
}

} // namespace repo
