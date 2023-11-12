/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2023, Regents of the University of California.
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

#include "tcp-bulk-insert-handle.hpp"

#include <boost/asio/ip/v6_only.hpp>

#include <ndn-cxx/util/logger.hpp>

NDN_LOG_INIT(repo.TcpHandle);

namespace ip = boost::asio::ip;

namespace repo {
namespace {

class TcpBulkInsertClient : noncopyable
{
public:
  TcpBulkInsertClient(TcpBulkInsertHandle& writer, ip::tcp::socket socket)
    : m_writer(writer)
    , m_socket(std::move(socket))
  {
  }

  static void
  startReceive(TcpBulkInsertHandle& writer, ip::tcp::socket socket)
  {
    auto client = std::make_shared<TcpBulkInsertClient>(writer, std::move(socket));
    client->m_socket.async_receive(
      boost::asio::buffer(client->m_inputBuffer, ndn::MAX_NDN_PACKET_SIZE), 0,
      std::bind(&TcpBulkInsertClient::handleReceive, client, _1, _2, client));
  }

private:
  void
  handleReceive(const boost::system::error_code& error, std::size_t nBytesReceived,
                const std::shared_ptr<TcpBulkInsertClient>& client);

private:
  TcpBulkInsertHandle& m_writer;
  ip::tcp::socket m_socket;
  uint8_t m_inputBuffer[ndn::MAX_NDN_PACKET_SIZE];
  std::size_t m_inputBufferSize = 0;
};

} // namespace

TcpBulkInsertHandle::TcpBulkInsertHandle(boost::asio::io_context& io,
                                         RepoStorage& storageHandle)
  : m_acceptor(io)
  , m_storageHandle(storageHandle)
{
}

void
TcpBulkInsertHandle::listen(const std::string& host, const std::string& port)
{
  ip::tcp::resolver resolver(m_acceptor.get_executor());
  boost::system::error_code ec;
  auto results = resolver.resolve(host, port, ec);
  if (ec)
    NDN_THROW(Error("Cannot resolve " + host + ":" + port + " (" + ec.message() + ")"));

  m_localEndpoint = *results.begin();
  NDN_LOG_DEBUG("Start listening on " << m_localEndpoint);

  m_acceptor.open(m_localEndpoint.protocol());
  m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
  if (m_localEndpoint.address().is_v6()) {
    m_acceptor.set_option(ip::v6_only(true));
  }
  m_acceptor.bind(m_localEndpoint);
  m_acceptor.listen();

  asyncAccept();
}

void
TcpBulkInsertHandle::stop()
{
  m_acceptor.cancel();
  m_acceptor.close();
}

void
TcpBulkInsertHandle::asyncAccept()
{
  m_acceptor.async_accept([this] (const auto& error, ip::tcp::socket socket) {
    if (error) {
      return;
    }

    NDN_LOG_DEBUG("New connection from " << socket.remote_endpoint());
    TcpBulkInsertClient::startReceive(*this, std::move(socket));

    // prepare accepting the next connection
    asyncAccept();
  });
}

void
TcpBulkInsertClient::handleReceive(const boost::system::error_code& error,
                                   std::size_t nBytesReceived,
                                   const std::shared_ptr<TcpBulkInsertClient>& client)
{
  if (error) {
    if (error == boost::asio::error::operation_aborted) // when socket is closed by someone
      return;

    boost::system::error_code ec;
    m_socket.shutdown(ip::tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
    return;
  }

  m_inputBufferSize += nBytesReceived;

  // do magic

  auto bufferView = ndn::make_span(m_inputBuffer, m_inputBufferSize);
  std::size_t offset = 0;
  bool isOk = true;
  while (offset < bufferView.size()) {
    Block element;
    std::tie(isOk, element) = Block::fromBuffer(bufferView.subspan(offset));
    if (!isOk)
      break;

    offset += element.size();
    BOOST_ASSERT(offset <= bufferView.size());

    if (element.type() == ndn::tlv::Data) {
      try {
        Data data(element);
        bool isInserted = m_writer.getStorageHandle().insertData(data);
        if (isInserted)
          NDN_LOG_DEBUG("Successfully injected " << data.getName());
        else
          NDN_LOG_DEBUG("FAILED to inject " << data.getName());
      }
      catch (const std::runtime_error& e) {
        /// \todo Catch specific error after determining what wireDecode() can throw
        NDN_LOG_ERROR("Error decoding received Data packet: " << e.what());
      }
    }
  }

  if (!isOk && m_inputBufferSize == ndn::MAX_NDN_PACKET_SIZE && offset == 0) {
    boost::system::error_code ec;
    m_socket.shutdown(ip::tcp::socket::shutdown_both, ec);
    m_socket.close(ec);
    return;
  }

  if (offset > 0) {
    if (offset != m_inputBufferSize) {
      std::copy(m_inputBuffer + offset, m_inputBuffer + m_inputBufferSize, m_inputBuffer);
      m_inputBufferSize -= offset;
    }
    else {
      m_inputBufferSize = 0;
    }
  }

  m_socket.async_receive(boost::asio::buffer(m_inputBuffer + m_inputBufferSize,
                                             ndn::MAX_NDN_PACKET_SIZE - m_inputBufferSize), 0,
                         std::bind(&TcpBulkInsertClient::handleReceive, this, _1, _2, client));
}

} // namespace repo
