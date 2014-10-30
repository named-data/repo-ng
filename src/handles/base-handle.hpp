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

#ifndef REPO_HANDLES_BASE_HANDLE_HPP
#define REPO_HANDLES_BASE_HANDLE_HPP

#include "common.hpp"

#include "storage/repo-storage.hpp"
#include "repo-command-response.hpp"
#include "repo-command-parameter.hpp"

namespace repo {

class BaseHandle : noncopyable
{
public:
  class Error : std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

public:
  BaseHandle(Face& face, RepoStorage& storageHandle, KeyChain& keyChain,
             Scheduler& scheduler)
    : m_face(face)
    , m_storageHandle(storageHandle)
    , m_keyChain(keyChain)
    , m_scheduler(scheduler)
   // , m_storeindex(storeindex)
  {
  }

  virtual void
  listen(const Name& prefix) = 0;

protected:

  inline Face&
  getFace()
  {
    return m_face;
  }

  inline RepoStorage&
  getStorageHandle()
  {
    return m_storageHandle;
  }

  inline Scheduler&
  getScheduler()
  {
    return m_scheduler;
  }

  // inline RepoStorage&
  // getStoreIndex()
  // {
  //   return m_storeindex;
  // }

  uint64_t
  generateProcessId();

  void
  reply(const Interest& commandInterest, const RepoCommandResponse& response);


  /**
   * @brief extract RepoCommandParameter from a command Interest.
   * @param interest command Interest
   * @param prefix Name prefix up to command-verb
   * @param[out] parameter parsed parameter
   * @throw RepoCommandParameter::Error parse error
   */
  void
  extractParameter(const Interest& interest, const Name& prefix, RepoCommandParameter& parameter);

private:

  Face& m_face;
  RepoStorage& m_storageHandle;
  KeyChain& m_keyChain;
  Scheduler& m_scheduler;
 // RepoStorage& m_storeindex;
};

inline void
BaseHandle::reply(const Interest& commandInterest, const RepoCommandResponse& response)
{
  std::shared_ptr<Data> rdata = std::make_shared<Data>(commandInterest.getName());
  rdata->setContent(response.wireEncode());
  m_keyChain.sign(*rdata);
  m_face.put(*rdata);
}

inline void
BaseHandle::extractParameter(const Interest& interest, const Name& prefix,
                             RepoCommandParameter& parameter)
{
  parameter.wireDecode(interest.getName().get(prefix.size()).blockFromValue());
}

} // namespace repo

#endif // REPO_HANDLES_BASE_HANDLE_HPP
