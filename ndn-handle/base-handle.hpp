/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_NDN_HANDLE_BASE_HANDLE_HPP
#define REPO_NDN_HANDLE_BASE_HANDLE_HPP

#include "ndn-handle-common.hpp"

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
  BaseHandle(Face& face, StorageHandle& storageHandle, KeyChain& keyChain, Scheduler& scheduler)
    : m_face(face)
    , m_storageHandle(storageHandle)
    , m_keyChain(keyChain)
    , m_scheduler(scheduler)
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

  inline StorageHandle&
  getStorageHandle()
  {
    return m_storageHandle;
  }

  inline Scheduler&
  getScheduler()
  {
    return m_scheduler;
  }

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
  StorageHandle& m_storageHandle;
  KeyChain& m_keyChain;
  Scheduler& m_scheduler;
};

inline void
BaseHandle::reply(const Interest& commandInterest, const RepoCommandResponse& response)
{
  Data rdata(commandInterest.getName());
  rdata.setContent(response.wireEncode());
  m_keyChain.sign(rdata);
  m_face.put(rdata);
}

inline void
BaseHandle::extractParameter(const Interest& interest, const Name& prefix,
                             RepoCommandParameter& parameter)
{
  parameter.wireDecode(interest.getName().get(prefix.size()).blockFromValue());
}

} //namespace repo

#endif // REPO_NDN_HANDLE_BASE_HANDLE_HPP
