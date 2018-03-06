/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018, Regents of the University of California.
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

#include "repo-command-response.hpp"

#include "common.hpp"

#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(RepoCommandResponse)

BOOST_AUTO_TEST_CASE(EncodeDecode)
{
  repo::RepoCommandResponse response;
  response.setCode(404);
  response.setStartBlockId(1);
  response.setEndBlockId(100);
  response.setProcessId(1234567890);
  response.setInsertNum(100);
  response.setDeleteNum(100);

  Block wire = response.wireEncode();

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  // Construct a \c Block from hexadecimal \p input.

  Block expected = "CF16 CE04499602D2D0020194CC0101CD0164D10164D20164"_block;

  BOOST_CHECK_EQUAL(wire, expected);

  repo::RepoCommandResponse decoded(wire);
  BOOST_CHECK_EQUAL(decoded.getCode(), response.getCode());
  BOOST_CHECK_EQUAL(decoded.getStartBlockId(), response.getStartBlockId());
  BOOST_CHECK_EQUAL(decoded.getEndBlockId(), response.getEndBlockId());
  BOOST_CHECK_EQUAL(decoded.getProcessId(), response.getProcessId());
  BOOST_CHECK_EQUAL(decoded.getInsertNum(), response.getInsertNum());
  BOOST_CHECK_EQUAL(decoded.getDeleteNum(), response.getDeleteNum());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
