/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "../helpers/repo-command-response.hpp"

#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(RepoCommandResponse)

BOOST_AUTO_TEST_CASE(EncodeDecode)
{
  repo::RepoCommandResponse response;
  response.setStatusCode(404);
  response.setStartBlockId(1);
  response.setEndBlockId(100);
  response.setProcessId(1234567890);
  response.setInsertNum(100);
  response.setDeleteNum(100);

  ndn::Block wire = response.wireEncode();

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  //for (ndn::Buffer::const_iterator it = wire.begin(); it != wire.end(); ++it) {
  //  printf("0x%02x, ", *it);
  //}
  static const uint8_t expected[] = {
    0xcf, 0x16, 0xce, 0x04, 0x49, 0x96, 0x02, 0xd2, 0xd0, 0x02,
    0x01, 0x94, 0xcc, 0x01, 0x01, 0xcd, 0x01, 0x64, 0xd1, 0x01,
    0x64, 0xd2, 0x01, 0x64
  };

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected, expected + sizeof(expected),
                                  wire.begin(), wire.end());

  BOOST_REQUIRE_NO_THROW(repo::RepoCommandResponse(wire));

  repo::RepoCommandResponse decoded(wire);
  BOOST_CHECK_EQUAL(decoded.getStatusCode(), response.getStatusCode());
  BOOST_CHECK_EQUAL(decoded.getStartBlockId(), response.getStartBlockId());
  BOOST_CHECK_EQUAL(decoded.getEndBlockId(), response.getEndBlockId());
  BOOST_CHECK_EQUAL(decoded.getProcessId(), response.getProcessId());
  BOOST_CHECK_EQUAL(decoded.getInsertNum(), response.getInsertNum());
  BOOST_CHECK_EQUAL(decoded.getDeleteNum(), response.getDeleteNum());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
