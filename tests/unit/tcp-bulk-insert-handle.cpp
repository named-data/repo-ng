/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023,  Regents of the University of California.
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

#include "handles/tcp-bulk-insert-handle.hpp"

#include "../repo-storage-fixture.hpp"
#include "../dataset-fixtures.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/test/unit_test.hpp>

namespace repo::tests {

BOOST_AUTO_TEST_SUITE(TcpBulkInsertHandle)

class TcpClient
{
public:
  void
  start(const std::string& host, const std::string& port)
  {
    boost::asio::ip::tcp::resolver resolver(ioCtx);
    boost::system::error_code ec;
    auto results = resolver.resolve(host, port, ec);
    if (ec) {
      BOOST_FAIL("Cannot resolve [" + host + ":" + port + "]");
    }

    socket.async_connect(*results.begin(), std::bind(&TcpClient::handleConnect, this, _1));
  }

  virtual void
  handleConnect(const boost::system::error_code& error)
  {
    if (error) {
      BOOST_FAIL("TCP connection failed");
    }
  }

public:
  boost::asio::io_context ioCtx;
  boost::asio::ip::tcp::socket socket{ioCtx};
};

template<class Dataset>
class TcpBulkInsertFixture : public TcpClient,
                             public RepoStorageFixture,
                             public Dataset
{
public:
  TcpBulkInsertFixture()
    : scheduler(ioCtx)
    , bulkInserter(ioCtx, *handle)
  {
    guardEvent = scheduler.schedule(2_s, std::bind(&TcpBulkInsertFixture::fail, this, "Test timed out"));
  }

  void
  handleConnect(const boost::system::error_code& error) override
  {
    TcpClient::handleConnect(error);

    // This value may need to be adjusted if some dataset exceeds 100k
    socket.set_option(boost::asio::socket_base::send_buffer_size(100000));

    // Initially I wrote the following to use scatter-gather approach (using
    // std::vector<const_buffer> and a single socket.async_send operation). Unfortunately, as
    // described in http://www.boost.org/doc/libs/1_48_0/doc/html/boost_asio/overview/implementation.html,
    // scatter-gather is limited to at most `min(64,IOV_MAX)` buffers to be transmitted
    // in a single operation
    for (auto i = this->data.begin(); i != this->data.end(); ++i) {
      socket.async_send(boost::asio::buffer((*i)->wireEncode()),
                        std::bind(&TcpBulkInsertFixture::onSendFinished, this, _1, false));
    }
    onSendFinished(error, true);
  }

  void
  onSendFinished(const boost::system::error_code& error, bool isFinal)
  {
    if (error) {
      BOOST_FAIL("TCP connection aborted");
      return;
    }

    if (isFinal) {
      guardEvent.cancel();

      // In case there are some outstanding handlers
      scheduler.schedule(1_s, [this] { stop(); });
    }
  }

  void
  fail(const std::string& info)
  {
    ioCtx.stop();
    BOOST_FAIL(info);
  }

  void
  stop()
  {
    // Terminate test
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket.close();

    bulkInserter.stop();
    // may be ioCtx.stop() as well
  }

public:
  Scheduler scheduler;
  ndn::scheduler::EventId guardEvent;
  repo::TcpBulkInsertHandle bulkInserter;
};

BOOST_FIXTURE_TEST_CASE_TEMPLATE(BulkInsertAndRead, T, CommonDatasets, TcpBulkInsertFixture<T>)
{
  // start bulk inserter
  this->bulkInserter.listen("localhost", "17376");

  // start test
  this->start("localhost", "17376");

  // actually run the test
  this->ioCtx.run();

  // Read (all items should exist)
  for (auto i = this->interests.begin(); i != this->interests.end(); ++i) {
    BOOST_CHECK_EQUAL(*this->handle->readData(i->first), *i->second);
  }
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace repo::tests
