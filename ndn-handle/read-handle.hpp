/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_NDN_HANDLE_READ_HANDLE_HPP
#define REPO_NDN_HANDLE_READ_HANDLE_HPP

#include "base-handle.hpp"

namespace repo {

class ReadHandle : BaseHandle
{

public:
  ReadHandle(Face* face, StorageHandle* storageHandle)
  : BaseHandle(face, storageHandle)
  {
  }

  virtual void
  listen(const Name& prefix);

private:
  /**
   * @brief Read the name from backend storage
   */
  void
  onInterest(const Name& prefix, const Interest& interest);

  void
  onRegisterFailed(const Name& prefix, const std::string& reason);
};

} //namespace repo

#endif // REPO_NDN_HANDLE_READ_HANDLE_HPP
