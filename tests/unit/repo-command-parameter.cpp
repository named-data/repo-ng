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

#include "repo-command-parameter.hpp"
#include "common.hpp"

#include <ndn-cxx/encoding/block-helpers.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

namespace repo::tests {

BOOST_AUTO_TEST_SUITE(RepoCommandParameter)

BOOST_AUTO_TEST_CASE(EncodeDecode)
{
  repo::RepoCommandParameter parameter;
  parameter.setName("/a/b/c");
  parameter.setStartBlockId(1);
  parameter.setEndBlockId(100);
  parameter.setProcessId(1234567890);

  Block wire = parameter.wireEncode();

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  // Construct a \c Block from hexadecimal \p input.
  Block expected = "C917 0709080161080162080163CC0101CD0164CE04499602D2"_block;

  BOOST_CHECK_EQUAL(wire, expected);

  repo::RepoCommandParameter decoded(wire);
  BOOST_CHECK_EQUAL(decoded.getName(), parameter.getName());
  BOOST_CHECK_EQUAL(decoded.getStartBlockId(), parameter.getStartBlockId());
  BOOST_CHECK_EQUAL(decoded.getEndBlockId(), parameter.getEndBlockId());
  BOOST_CHECK_EQUAL(decoded.getProcessId(), parameter.getProcessId());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace repo::tests
