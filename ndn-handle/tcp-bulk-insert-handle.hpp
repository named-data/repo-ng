/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_NDN_HANDLE_WRITE_TCP_BACKDOOR_HPP
#define REPO_NDN_HANDLE_WRITE_TCP_BACKDOOR_HPP

#include "ndn-handle-common.hpp"
#include <boost/asio.hpp>

namespace repo {

class TcpBulkInsertHandle : noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  TcpBulkInsertHandle(boost::asio::io_service& ioService,
                      StorageHandle& storageHandle);

  void
  listen(const std::string& host, const std::string& port);

  void
  stop();

  StorageHandle&
  getStorageHandle()
  {
    return m_storageHandle;
  }

private:
  void
  handleAccept(const boost::system::error_code& error,
               const shared_ptr<boost::asio::ip::tcp::socket>& socket);

private:
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::tcp::endpoint m_localEndpoint;
  StorageHandle& m_storageHandle;
};

} // namespace repo

#endif // REPO_NDN_HANDLE_WRITE_TCP_BACKDOOR_HPP
