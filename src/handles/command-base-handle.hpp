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

#ifndef REPO_HANDLES_COMMAND_BASE_HANDLE_HPP
#define REPO_HANDLES_COMMAND_BASE_HANDLE_HPP

#include "common.hpp"

#include "storage/repo-storage.hpp"
#include "repo-command-response.hpp"
#include "repo-command-parameter.hpp"
#include "repo-command.hpp"

#include <ndn-cxx/mgmt/dispatcher.hpp>
#include <ndn-cxx/security/validator.hpp>

namespace repo {

class CommandBaseHandle
{
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

public:
  CommandBaseHandle(Face& face, RepoStorage& storageHandle,
                    Scheduler& scheduler, ndn::security::Validator& validator);

  virtual
  ~CommandBaseHandle() = default;

  ndn::mgmt::Authorization
  makeAuthorization();

  template<typename T>
  bool
  validateParameters(const ndn::mgmt::ControlParameters& parameters)
  {
    const auto* castParams = dynamic_cast<const RepoCommandParameter*>(&parameters);
    BOOST_ASSERT(castParams != nullptr);

    T command;
    try {
      command.validateRequest(*castParams);
    }
    catch (const RepoCommand::ArgumentError&) {
      return false;
    }
    return true;
  }

protected:
  Face& face;
  RepoStorage& storageHandle;
  Scheduler& scheduler;

private:
  ndn::security::Validator& m_validator;
};

} // namespace repo

#endif // REPO_HANDLES_COMMAND_BASE_HANDLE_HPP
