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

#ifndef REPO_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP
#define REPO_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP

#include "common.hpp"

#include <ndn-cxx/security/key-chain.hpp>

namespace repo::tests {

/** \brief a fixture providing an in-memory KeyChain
 */
class IdentityManagementFixture : noncopyable
{
public:
  IdentityManagementFixture();

  /** \brief deletes saved certificate files
   */
  ~IdentityManagementFixture();

  /** \brief add identity
   *  \return whether successful
   */
  bool
  addIdentity(const Name& identity,
              const ndn::KeyParams& params = ndn::KeyChain::getDefaultKeyParams());

  /** \brief save identity certificate to a file
   *  \param identity identity name
   *  \param filename file name, should be writable
   *  \param wantAdd if true, add new identity when necessary
   *  \return whether successful
   */
  bool
  saveIdentityCertificate(const Name& identity, const std::string& filename, bool wantAdd = false);

protected:
  ndn::KeyChain m_keyChain;

private:
  std::vector<std::string> m_certFiles;
};

} // namespace repo::tests

#endif // REPO_TESTS_IDENTITY_MANAGEMENT_FIXTURE_HPP
