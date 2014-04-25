/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#include "../helpers/repo-command-parameter.hpp"
#include <ndn-cxx/selectors.hpp>

#include <boost/test/unit_test.hpp>

namespace repo {
namespace tests {

BOOST_AUTO_TEST_SUITE(RepoCommandParameter)

BOOST_AUTO_TEST_CASE(EncodeDecode)
{
  repo::RepoCommandParameter parameter;
  parameter.setName("/a/b/c");
  parameter.setStartBlockId(1);
  parameter.setEndBlockId(100);
  parameter.setProcessId(1234567890);
  ndn::Selectors selectors;
  selectors.setMaxSuffixComponents(1);
  parameter.setSelectors(selectors);

  ndn::Block wire = parameter.wireEncode();

  // These octets are obtained by the snippet below.
  // This check is intended to detect unexpected encoding change in the future.
  //for (ndn::Buffer::const_iterator it = wire.begin(); it != wire.end(); ++it) {
  //  printf("0x%02x, ", *it);
  //}
  static const uint8_t expected[] = {
    0xc9, 0x1c, 0x07, 0x09, 0x08, 0x01, 0x61, 0x08, 0x01, 0x62, 0x08,
    0x01, 0x63, 0x09, 0x03, 0x0e, 0x01, 0x01, 0xcc, 0x01, 0x01, 0xcd,
    0x01, 0x64, 0xce, 0x04, 0x49, 0x96, 0x02, 0xd2
  };

  BOOST_REQUIRE_EQUAL_COLLECTIONS(expected, expected + sizeof(expected),
                                  wire.begin(), wire.end());

  BOOST_REQUIRE_NO_THROW(repo::RepoCommandParameter(wire));

  repo::RepoCommandParameter decoded(wire);
  //std::cout << decoded << std::endl;
  BOOST_CHECK_EQUAL(decoded.getName(), parameter.getName());
  BOOST_CHECK_EQUAL(decoded.getStartBlockId(), parameter.getStartBlockId());
  BOOST_CHECK_EQUAL(decoded.getEndBlockId(), parameter.getEndBlockId());
  BOOST_CHECK_EQUAL(decoded.getProcessId(), parameter.getProcessId());
  BOOST_CHECK_EQUAL(decoded.getSelectors().getMaxSuffixComponents(),
                    parameter.getSelectors().getMaxSuffixComponents());
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace tests
} // namespace repo
