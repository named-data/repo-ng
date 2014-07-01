/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
* Copyright (c) 2014, Regents of the University of California.
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
* PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* repo-ng, e.g., in COPYING.md file. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REPO_STORAGE_SKIPLIST_HPP
#define REPO_STORAGE_SKIPLIST_HPP

#include "common.hpp"

#include <boost/utility.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace repo {

class SkipList32Layers25Probabilty
{
public:
  static size_t
  getMaxLayers()
  {
    return 32;
  }

  static double
  getProbability()
  {
    return 0.25; /* 25% */
  }
};

template<typename T, typename Compare = std::less<T>,
         typename Traits = SkipList32Layers25Probabilty >
class SkipList
{
public:
  //@brief layer of skiplist is std::list
  typedef std::list<T> SkipListLayer;
  //@brief iterator of skiplist
  typedef typename SkipListLayer::iterator iterator;

public:
  explicit
  SkipList();

  ~SkipList();

  size_t
  size() const;

public: // enumeration

  iterator
  begin() const;

  iterator
  end() const;

  iterator
  find(const T& key);

  iterator
  lower_bound(const T& key);

  std::pair<iterator, bool>
  insert(const T& key);

  iterator
  erase(iterator it);

private:
  size_t
  pickRandomLayer() const;

private:
  //@brief store multiple layers
  typedef std::list<shared_ptr<SkipListLayer> > SkipListHeadLayer;
  //@brief iterate one layer of skiplist
  typedef typename SkipListLayer::iterator layer_iterator;
  //@brief iterate among different layers
  typedef typename SkipListHeadLayer::iterator head_iterator;
  typedef typename SkipListHeadLayer::reverse_iterator head_reverse_iterator;

private:
  SkipListHeadLayer m_skipList;
  //@brief keep the locations of iterator where element is inserted in each layer
  //key of external map is the layer of skiplist
  //value is the internal map which maps the key T with the iterator of each layer
  std::map<size_t, std::map<T, layer_iterator, Compare> > m_layerToEntry;
  Compare m_compare;
};

template<typename T, typename Compare, typename Traits>
inline
SkipList<T, Compare, Traits>::SkipList()
{
  shared_ptr<SkipListLayer> zeroLayer = make_shared<SkipListLayer>();
  m_skipList.push_back(zeroLayer);
}

template<typename T, typename Compare, typename Traits>
inline
SkipList<T, Compare, Traits>::~SkipList()
{
}

template<typename T, typename Compare, typename Traits>
inline size_t
SkipList<T, Compare, Traits>::size() const
{
  return (*m_skipList.begin())->size();
}

template<typename T, typename Compare, typename Traits>
inline typename SkipList<T, Compare, Traits>::iterator
SkipList<T, Compare, Traits>::begin() const
{
   return (*m_skipList.begin())->begin();
}

template<typename T, typename Compare, typename Traits>
inline typename SkipList<T, Compare, Traits>::iterator
SkipList<T, Compare, Traits>::end() const
{
  return (*m_skipList.begin())->end();
}

template<typename T, typename Compare, typename Traits>
inline typename SkipList<T, Compare, Traits>::iterator
SkipList<T, Compare, Traits>::find(const T& key)
{
  iterator it = this->lower_bound(key);
  if (it == this->end() || *it != key)
    return this->end();
  return it;
}

template<typename T, typename Compare, typename Traits>
inline typename SkipList<T, Compare, Traits>::iterator
SkipList<T, Compare, Traits>::lower_bound(const T& key)
{
  bool isIterated = false;
  bool isIdentical = false;
  head_reverse_iterator topLayer = m_skipList.rbegin();
  layer_iterator head = (*topLayer)->begin();

  if (!(*topLayer)->empty()) {
    size_t layer = m_skipList.size() - 1;
    for (head_reverse_iterator headReverseIterator = topLayer;
         headReverseIterator != m_skipList.rend(); ++headReverseIterator) {
      if (!isIterated)
        head = (*headReverseIterator)->begin();

      if (head != (*headReverseIterator)->end()) {
        if (!isIterated && (!m_compare(*head, key) && !m_compare(key, *head))) {
          if (layer > 0) {
            layer--;
            continue; // try lower layer
          }
          else {
            isIterated = true;
            isIdentical = true;
          }
        }
        else {
          layer_iterator layerIterator = head;
          while (m_compare((*layerIterator) , key)) {
            head = layerIterator;
            isIterated = true;

            ++layerIterator;
            if (layerIterator == (*headReverseIterator)->end())
              break;
          }

        }
      }

      if (layer > 0) {
        //find the head in the lower layer
        head = m_layerToEntry[layer - 1][*head];
      }
      else {  //if we reached the first layer
        if (isIterated) {
          if (!isIdentical)
            ++head;
          //result = head;
          return head;
        }
        else {
          return head;
        }
      }

      layer--;
    }
  }
  return (*m_skipList.begin())->end();
}

template<typename T, typename Compare, typename Traits>
inline size_t
SkipList<T, Compare, Traits>::pickRandomLayer() const
{
  static boost::random::mt19937 gen;
  static boost::random::geometric_distribution<size_t> dist(Traits::getProbability());
  return std::min(dist(gen), Traits::getMaxLayers());
}

template<typename T, typename Compare, typename Traits>
inline std::pair<typename SkipList<T, Compare, Traits>::iterator ,bool>
SkipList<T, Compare, Traits>::insert(const T& key)
{
  bool insertInFront = false;
  bool isIterated = false;
  head_reverse_iterator topLayer = m_skipList.rbegin();
  std::vector<layer_iterator> updateTable(Traits::getMaxLayers());
  layer_iterator head = (*topLayer)->begin();

  if (!(*topLayer)->empty()) {
    size_t layer = m_skipList.size() - 1;
    for (head_reverse_iterator headReverseIterator = topLayer;
         headReverseIterator != m_skipList.rend(); ++headReverseIterator) {
      if (!isIterated)
        head = (*headReverseIterator)->begin();

      updateTable[layer] = head;

      if (head != (*headReverseIterator)->end()) {
        if (!isIterated && !m_compare(*head, key)) {
          --updateTable[layer];
          insertInFront = true;
        }
        else {
          layer_iterator layerIterator = head;

          while (m_compare(*layerIterator , key)) {
            head = layerIterator;
            updateTable[layer] = layerIterator;
            isIterated = true;

            ++layerIterator;
            if (layerIterator == (*headReverseIterator)->end())
              break;
          }

        }
      }

      if (layer > 0)
        head = m_layerToEntry[layer - 1][*head]; // move HEAD to the lower layer

      layer--;
    }
  }
  else {
    updateTable[0] = (*topLayer)->begin(); //initialization
  }

  head = updateTable[0];
  ++head;

  bool isInBoundaries = (head != (*m_skipList.begin())->end());
  bool isKeyIdentical = false;
  if (isInBoundaries) {
    isKeyIdentical = (!m_compare(*head, key) && !m_compare(key, *head));
  }
  if (isKeyIdentical) {
    return std::make_pair(head, false);
  }

  size_t randomLayer = pickRandomLayer();

  while (m_skipList.size() < randomLayer + 1) {
     boost::shared_ptr<SkipListLayer> newLayer = make_shared<SkipListLayer>();
    m_skipList.push_back(newLayer);

    updateTable[(m_skipList.size() - 1)] = newLayer->begin();
  }

  size_t layer = 0;
  layer_iterator result;

  for (head_iterator headIterator = m_skipList.begin();
       headIterator != m_skipList.end() && layer <= randomLayer; ++headIterator) {
    if (updateTable[layer] == (*headIterator)->end() && !insertInFront) {
      (*headIterator)->push_back(key);
      layer_iterator last = (*headIterator)->end();
      --last;
      m_layerToEntry[layer][key] = last;

    }
    else if (updateTable[layer] == (*headIterator)->end() && insertInFront) {
      (*headIterator)->push_front(key);
      m_layerToEntry[layer][key] = (*headIterator)->begin();
    }
    else {
      ++updateTable[layer]; // insert after
      iterator position = (*headIterator)->insert(updateTable[layer], key);
      m_layerToEntry[layer][key] = position;  // save iterator where item was inserted
    }
    if (layer == 0)
      result = m_layerToEntry[layer][key];
    layer++;
  }
  return make_pair(result, true);
}

template<typename T, typename Compare, typename Traits>
inline typename SkipList<T, Compare, Traits>::iterator
SkipList<T, Compare, Traits>::erase(typename SkipList<T, Compare, Traits>::iterator it)
{
  if (it != end()) {
    int layer = 1;
    head_iterator headIterator = m_skipList.begin();
    headIterator++;
    for (; headIterator != m_skipList.end();) {
      const std::map<T, layer_iterator, Compare>& layerIterators = m_layerToEntry[layer];
      if(!layerIterators.empty()) {
        typename std::map<T, layer_iterator, Compare>::const_iterator eraseIterator =
          layerIterators.find(*it);
        if (eraseIterator != layerIterators.end()) {
          (*headIterator)->erase(eraseIterator->second);
          m_layerToEntry[layer].erase(*it);
          //remove layers that do not contain any elements (starting from the second layer)
          if ((*headIterator)->empty()) {
            // delete headIterator;
            headIterator = m_skipList.erase(headIterator);
          }
          else {
            ++headIterator;
          }
          layer++;
        }
        else {
          break;
        }
      }
    }

    m_layerToEntry[0].erase(*it);
    m_skipList.end();
    typename SkipList<T, Compare, Traits>::iterator returnIterator =
      (*m_skipList.begin())->erase(it);

    return returnIterator;
  }
  else {
    return end();
  }
}

} // namespace repo

#endif // REPO_STORAGE_SKIPLIST_HPP
