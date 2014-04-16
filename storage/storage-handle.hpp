/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (C) 2014 Regents of the University of California.
 * See COPYING for copyright and distribution information.
 */

#ifndef REPO_STORAGE_STORAGE_HANDLE_HPP
#define REPO_STORAGE_STORAGE_HANDLE_HPP

#include <string>
#include <stdexcept>

#include <ndn-cpp-dev/interest.hpp>
#include <ndn-cpp-dev/name.hpp>
#include <ndn-cpp-dev/data.hpp>
#include <ndn-cpp-dev/selectors.hpp>
#include <ndn-cpp-dev/key-locator.hpp>

#include "storage-method.hpp"

namespace repo {

using ndn::Interest;
using ndn::Name;
using ndn::Data;
using ndn::Selectors;
using ndn::KeyLocator;
using ndn::Block;
using ndn::Exclude;

using std::vector;
using std::string;
using boost::noncopyable;

/**
 * @brief this class defines handles to read, insert and delete data packets in storage media
 */

class StorageHandle : noncopyable
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

  /**
   * @brief Create a basic class object the specified storage type
   */
  explicit
  StorageHandle(StorageMethod storageMethod);

  virtual
  ~StorageHandle();

  /**
   * @return storage method defined in storage-define.hpp
   */
  StorageMethod
  getStorageMethod() const;

  /**
   * @brief insert data
   * @return true on success, false otherwise
   */
  virtual bool
  insertData(const Data& data) = 0;

  /**
   * @brief delete the data that exactly matches the name
   * @return true on success, false otherwise
   * @note It's considered successful if Data doesn't exist.
   */
  virtual bool
  deleteData(const Name& name) = 0;

  /**
  * @brief find data according to the interest. This interest may contain selectors.
  * @param[out] data Data matching Interest.
  * @return true if Data is found, false otherwise
  */
  virtual bool
  readData(const Interest& interest, Data& data) = 0;

  /**
   * @return if storage media has data packets with this name, return true, else return false
   */
  virtual bool
  hasName(const Name& name) = 0;

  /**
   * @brief select any data conforms to the selector
   * @param[out] names Data names matching @p name and @p selectors.
   * @return true if at least one Data is found, false otherwise
   */
  virtual bool
  readNameAny(const Name& name, const Selectors& selectors, vector<Name>& names) = 0;

  /**
   * @brief Get the number of Data packets stored
   */
  virtual size_t
  size() = 0;

private:
  StorageMethod m_storageMethod;
};

inline
StorageHandle::StorageHandle(StorageMethod storageMethod)
  : m_storageMethod(storageMethod)
{
}

inline
StorageHandle::~StorageHandle()
{
}

inline StorageMethod
StorageHandle::getStorageMethod() const
{
  return m_storageMethod;
}

} // namespace repo

#endif // REPO_STORAGE_STORAGE_HANDLE_HPP
