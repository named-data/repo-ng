/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California.
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

namespace repo {

const size_t MAX_NDN_PACKET_SIZE = 8800;

namespace detail {

class TcpBulkInsertClient : noncopyable
{
public:
  TcpBulkInsertClient(TcpBulkInsertHandle& writer,
                      const shared_ptr<boost::asio::ip::tcp::socket>& socket)
    : m_writer(writer)
    , m_socket(socket)
    , m_hasStarted(false)
    , m_inputBufferSize(0)
  {
  }

  static void
  startReceive(const shared_ptr<TcpBulkInsertClient>& client)
  {
    BOOST_ASSERT(!client->m_hasStarted);

    client->m_socket->async_receive(
      boost::asio::buffer(client->m_inputBuffer, MAX_NDN_PACKET_SIZE), 0,
      bind(&TcpBulkInsertClient::handleReceive, client, _1, _2, client));

    client->m_hasStarted = true;
  }

private:
  void
  handleReceive(const boost::system::error_code& error,
                std::size_t nBytesReceived,
                const shared_ptr<TcpBulkInsertClient>& client);

private:
  TcpBulkInsertHandle& m_writer;
  shared_ptr<boost::asio::ip::tcp::socket> m_socket;
  bool m_hasStarted;
  uint8_t m_inputBuffer[MAX_NDN_PACKET_SIZE];
  std::size_t m_inputBufferSize;
};

} // namespace detail

TcpBulkInsertHandle::TcpBulkInsertHandle(boost::asio::io_service& ioService,
                                         RepoStorage& storageHandle)
  : m_acceptor(ioService)
  , m_storageHandle(storageHandle)
{
}

void
TcpBulkInsertHandle::listen(const std::string& host, const std::string& port)
{
  using namespace boost::asio;

  ip::tcp::resolver resolver(m_acceptor.get_io_service());
  ip::tcp::resolver::query query(host, port);

  ip::tcp::resolver::iterator endpoint = resolver.resolve(query);
  ip::tcp::resolver::iterator end;

  if (endpoint == end)
    throw Error("Cannot listen on [" + host + ":" + port + "]");

  m_localEndpoint = *endpoint;
  std::cerr << "Start listening on " << m_localEndpoint  << std::endl;

  m_acceptor.open(m_localEndpoint .protocol());
  m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
  if (m_localEndpoint.address().is_v6())
    {
      m_acceptor.set_option(ip::v6_only(true));
    }
  m_acceptor.bind(m_localEndpoint);
  m_acceptor.listen(255);

  shared_ptr<ip::tcp::socket> clientSocket =
    make_shared<ip::tcp::socket>(boost::ref(m_acceptor.get_io_service()));
  m_acceptor.async_accept(*clientSocket,
                          bind(&TcpBulkInsertHandle::handleAccept, this, _1,
                               clientSocket));
}

void
TcpBulkInsertHandle::stop()
{
  m_acceptor.cancel();
  m_acceptor.close();
}

void
TcpBulkInsertHandle::handleAccept(const boost::system::error_code& error,
                                  const shared_ptr<boost::asio::ip::tcp::socket>& socket)
{
  using namespace boost::asio;

  if (error) {
    // if (error == boost::system::errc::operation_canceled) // when socket is closed by someone
    //   return;
    return;
  }

  std::cerr << "New connection from " << socket->remote_endpoint() << std::endl;

  shared_ptr<detail::TcpBulkInsertClient> client =
    make_shared<detail::TcpBulkInsertClient>(boost::ref(*this), socket);
  detail::TcpBulkInsertClient::startReceive(client);

  // prepare accepting the next connection
  shared_ptr<ip::tcp::socket> clientSocket =
    make_shared<ip::tcp::socket>(boost::ref(m_acceptor.get_io_service()));
  m_acceptor.async_accept(*clientSocket,
                          bind(&TcpBulkInsertHandle::handleAccept, this, _1,
                               clientSocket));
}

void
detail::TcpBulkInsertClient::handleReceive(const boost::system::error_code& error,
                                           std::size_t nBytesReceived,
                                           const shared_ptr<detail::TcpBulkInsertClient>& client)
{
  if (error)
    {
      if (error == boost::system::errc::operation_canceled) // when socket is closed by someone
        return;

      boost::system::error_code error;
      m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
      m_socket->close(error);
      return;
    }

  m_inputBufferSize += nBytesReceived;

  // do magic

  std::size_t offset = 0;

  bool isOk = true;
  Block element;
  while (m_inputBufferSize - offset > 0)
    {
      isOk = Block::fromBuffer(m_inputBuffer + offset, m_inputBufferSize - offset, element);
      if (!isOk)
        break;

      offset += element.size();
      BOOST_ASSERT(offset <= m_inputBufferSize);

      if (element.type() == ndn::tlv::Data)
        {
          try {
            Data data(element);
            bool isOk = m_writer.getStorageHandle().insertData(data);
            if (isOk)
              std::cerr << "Successfully injected " << data.getName() << std::endl;
            else
              std::cerr << "FAILED to inject " << data.getName() << std::endl;
          }
          catch (std::runtime_error& error) {
            /// \todo Catch specific error after determining what wireDecode() can throw
            std::cerr << "Error decoding received Data packet" << std::endl;
          }
        }
    }
  if (!isOk && m_inputBufferSize == MAX_NDN_PACKET_SIZE && offset == 0)
    {
      boost::system::error_code error;
      m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
      m_socket->close(error);
      return;
    }

  if (offset > 0)
    {
      if (offset != m_inputBufferSize)
        {
          std::copy(m_inputBuffer + offset, m_inputBuffer + m_inputBufferSize,
                    m_inputBuffer);
          m_inputBufferSize -= offset;
        }
      else
        {
          m_inputBufferSize = 0;
        }
    }

  m_socket->async_receive(boost::asio::buffer(m_inputBuffer + m_inputBufferSize,
                                              MAX_NDN_PACKET_SIZE - m_inputBufferSize), 0,
                          bind(&TcpBulkInsertClient::handleReceive, this, _1, _2, client));
}


} // namespace repo
