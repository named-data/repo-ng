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

#ifndef REPO_STORAGE_INDEX_HPP
#define REPO_STORAGE_INDEX_HPP

#include "common.hpp"
#include "skiplist.hpp"
#include <queue>

namespace repo {

class Index : noncopyable
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

  class Entry
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

    /**
     * @brief used by skiplist to construct node
     */
    Entry()
    {
    };

    /**
     * @brief construct Entry by data and id number
     */
    Entry(const Data& data, const int64_t id);

    /**
     * @brief construct Entry by fullName, keyLocator and id number
     */
    Entry(const Name& fullName, const KeyLocator& keyLocator, const int64_t id);

    /**
     * @brief construct Entry by fullName, keyLocatorHash and id number
     * @param  fullName        full name with digest computed from data
     * @param  keyLocatorHash  keyLocator hashed by sha256
     * @param  id              record ID from database
     */
    Entry(const Name& fullName, const ndn::ConstBufferPtr& keyLocatorHash, const int64_t id);

    /**
     *  @brief implicit construct Entry by full name
     *
     *  Allow implicit conversion from Name for SkipList lookups by Name
     */
    Entry(const Name& name);

    /**
     *  @brief get the name of entry
     */
    const Name&
    getName() const
    {
      return m_name;
    }

    /**
     *  @brief get the keyLocator hash value of the entry
     */
    const ndn::ConstBufferPtr&
    getKeyLocatorHash() const
    {
      return m_keyLocatorHash;
    }

    /**
     *  @brief get record ID from database
     */
    const int64_t
    getId() const
    {
      return m_id;
    }

    bool
    operator>(const Entry& entry) const
    {
      return m_name > entry.getName();
    }

    bool
    operator<(const Entry& entry) const
    {
      return m_name < entry.getName();
    }

    bool
    operator==(const Entry& entry) const
    {
      return m_name == entry.getName();
    }

    bool
    operator!=(const Entry& entry) const
    {
      return m_name != entry.getName();
    }

  private:
    Name m_name;
    ndn::ConstBufferPtr m_keyLocatorHash;
    int64_t m_id;
  };

private:

  typedef SkipList<Entry> IndexSkipList;

public:
  explicit
  Index(const size_t nMaxPackets);

  /**
   *  @brief insert entries into index
   *  @param  data    used to construct entries
   *  @param  id      obtained from database
   */
  bool
  insert(const Data& data, const int64_t id);

  /**
   *  @brief insert entries into index
   *  @param  data    used to construct entries
   *  @param  id      obtained from database
   */
  bool
  insert(const Name& fullName, const int64_t id,
         const ndn::ConstBufferPtr& keyLocatorHash);

  /**
   *  @brief erase the entry in index by its fullname
   */
  bool
  erase(const Name& fullName);

  /** @brief find the Entry for best match of an Interest
   * @return ID and fullName of the Entry, or (0,ignored) if not found
   */
  std::pair<int64_t, Name>
  find(const Interest& interest) const;

  /** @brief find the first Entry under a Name prefix
   * @return ID and fullName of the Entry, or (0,ignored) if not found
   */
  std::pair<int64_t, Name>
  find(const Name& name) const;

  /**
   *  @brief determine whether same Data is already in the index
   *  @return true if identical Data exists, false otherwise
   */
  bool
  hasData(const Data& data) const;

  /**
    *  @brief compute the hash value of keyLocator
    */
  static const ndn::ConstBufferPtr
  computeKeyLocatorHash(const KeyLocator& keyLocator);

  const size_t
  size() const
  {
    return m_size;
  }

private:
  /**
   *  @brief select entries which satisfy the selectors in interest and return their name
   *  @param  interest   used to select entries by comparing the name and checking selectors
   *  @param  idName    save the id and name of found entries
   *  @param  startingPoint the entry whose name is equal or larger than the interest name
   */
  std::pair<int64_t, Name>
  selectChild(const Interest& interest,
              IndexSkipList::const_iterator startingPoint) const;

  /**
   *  @brief check whether the index is full
   */
  bool
  isFull() const
  {
    return m_size >= m_maxPackets;
  }

  /**
   *  @brief find the first entry with the prefix
   *  @param  prefix        used to request the entries
   *  @param  startingPoint the entry whose name is equal or larger than the interest name
   *  @return int64_t  the id number of found entry
   *  @return Name     the name of found entry
   */
  std::pair<int64_t, Name>
  findFirstEntry(const Name& prefix,
                 IndexSkipList::const_iterator startingPoint) const;

private:
  IndexSkipList m_skipList;
  size_t m_maxPackets;
  size_t m_size;
};

} // namespace repo

#endif // REPO_STORAGE_INDEX_HPP
