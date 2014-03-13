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
  BaseHandle(Face* face, StorageHandle* storageHandle)
    : m_face(face)
    , m_storageHandle(storageHandle)
  {
  }

  virtual void
  listen(const Name& prefix) = 0;

  inline Face*
  getFace()
  {
    return m_face;
  }

  inline StorageHandle*
  getStorageHandle()
  {
    return m_storageHandle;
  }

private:
  Face* m_face;
  StorageHandle* m_storageHandle;
};

} //namespace repo

#endif // REPO_NDN_HANDLE_BASE_HANDLE_HPP
