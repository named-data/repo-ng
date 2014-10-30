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

#ifndef REPO_HANDLES_TCP_BULK_INSERT_HANDLE_HPP
#define REPO_HANDLES_TCP_BULK_INSERT_HANDLE_HPP

#include "common.hpp"
#include "storage/repo-storage.hpp"

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
                      RepoStorage& storageHandle);

  void
  listen(const std::string& host, const std::string& port);

  void
  stop();

  RepoStorage&
  getStorageHandle()
  {
    return m_storageHandle;
  }

private:
  void
  handleAccept(const boost::system::error_code& error,
               const std::shared_ptr<boost::asio::ip::tcp::socket>& socket);

private:
  boost::asio::ip::tcp::acceptor m_acceptor;
  boost::asio::ip::tcp::endpoint m_localEndpoint;
  RepoStorage& m_storageHandle;
};

} // namespace repo

#endif // REPO_HANDLES_TCP_BULK_INSERT_HANDLE_HPP
