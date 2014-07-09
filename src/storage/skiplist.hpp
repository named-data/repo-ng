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

namespace repo {

class SkipList32Levels25Probabilty
{
public:
  static size_t
  getMaxLevels()
  {
    return 32;
  }

  static double
  getProbability()
  {
    return 0.25;  // 25%
  }
};

template<typename T>
struct SkipListNode
{
  typedef SkipListNode<T>* SkipListNodePointer;
  T data;
  std::vector<SkipListNodePointer> prevs;
  std::vector<SkipListNodePointer> nexts;
};

template<typename T, class Ref, class Ptr>
class SkipListIterator : public std::iterator<std::bidirectional_iterator_tag, T>
{
public:
  typedef SkipListNode<T>* NodePointer;
  NodePointer node;

  typedef SkipListIterator<T, Ref, Ptr> Self;
  typedef T value_type;
  typedef Ptr pointer;
  typedef Ref reference;
  typedef ptrdiff_t difference_type;
  typedef SkipListIterator<T, const T&, const T*> const_iterator;
  /// alias of const_iterator
  typedef const_iterator iterator;

public:
  // constructor
  SkipListIterator()
  {
  }

  explicit
  SkipListIterator(NodePointer x)
    : node(x)
  {
  }

  SkipListIterator(const SkipListIterator& x)
    : node(x.node)
  {
  }

  bool
  operator==(const const_iterator& x) const
  {
    return node == x.node;
  }

  bool
  operator!=(const const_iterator& x) const
  {
    return node != x.node;
  }

  reference
  operator*() const
  {
    return (*node).data;
  }

  pointer
  operator->() const
  {
    return &(operator*());
  }

  Self&
  operator++()
  {
    node = node->nexts[0];
    return *this;
  }

  Self
  operator++(int)
  {
    Self tmp = *this;
    ++*this;
    return tmp;
  }

  Self&
  operator--()
  {
    node = node->prevs[0];
    return *this;
  }

  Self
  operator--(int)
  {
    Self tmp = *this;
    --*this;
    return tmp;
  }
};

/*
 * @brief SkipList
 *
 * Examples of internal structure:
 * "A <-i-> B" means A->nexts[i] = B and B->prevs[i] = A
 *
 * case 1: an empty skip list
 * m_head <-0-> m_head
 *
 * case 2: a skip list with only one level and only one node
 * m_head <-0-> A <-0-> m_head
 *
 * case 3: a skip list with three nodes, node A and B appear on one level,
 * node C appears on two levels
 * m_head         <-1->         C <-1-> m_head
 * m_head <-0-> A <-0-> B <-0-> C <-0-> m_head
 */

template<typename T, typename Compare = std::less<T>,
         typename Traits = SkipList32Levels25Probabilty>
class SkipList
{
public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef SkipListNode<T> Node;
  typedef Node* NodePointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef SkipListIterator<T, const T&, const T*> const_iterator;
  /// alias of const_iterator
  typedef const_iterator iterator;

public:
  explicit
  SkipList()
  {
    initializeHead();
  }

  ~SkipList()
  {
    clear();
    deallocateNode(m_head);
  }

  const_iterator
  begin() const
  {
    return const_iterator(*(*m_head).nexts.begin());
  }

  const_iterator
  end() const
  {
    return const_iterator(m_head);
  }

  bool
  empty() const
  {
    return *(m_head->nexts.begin()) == m_head;
  }

  size_t
  size() const
  {
    return m_size;
  }

  const_iterator
  lower_bound(const T& x) const;

  const_iterator
  find(const T& x) const;

  std::pair<const_iterator, bool>
  insert(const T& x);

  const_iterator
  erase(const_iterator it);

protected:
  /*
   * @brief allocate memory for node
   */
  NodePointer
  allocateNode()
  {
    return m_skiplistAllocator.allocate(sizeof(Node));
  }

  /*
   * @brief deallocate memory of node
   */
  void
  deallocateNode(NodePointer p)
  {
    m_skiplistAllocator.deallocate(p, sizeof(Node));
  }

  /*
   * @brief initialize the node
   */
  NodePointer
  createNode()
  {
    NodePointer p = allocateNode();
    Node node;
    m_skiplistAllocator.construct(p, node);
    return p;
  }

  /*
   * @brief initialize the node with given value
   * @para to be set to the value of node
   */
  NodePointer
  createNode(const T& x)
  {
    NodePointer p = allocateNode();
    Node node;
    m_skiplistAllocator.construct(p, node);
    m_dataAllocator.construct(&(p->data), x);
    return p;
  }

  /*
   * @brief destructror of the node
   * @para given pointer of node to be destructed
   */
  void
  destroyNode(NodePointer p)
  {
    m_skiplistAllocator.destroy(p);
    deallocateNode(p);
  }

  /*
   * @brief initialize the head
   */
  void
  initializeHead()
  {
    m_head = createNode();
    m_head->prevs.push_back(m_head);
    m_head->nexts.push_back(m_head);
    m_size = 0;
  }

  /*
   * @brief destroy all the nodes of skiplist except the head
   */
  void
  clear()
  {
    NodePointer cur = m_head->nexts[0];
    while (cur != m_head) {
      NodePointer tmp = cur;
      cur = cur->nexts[0];
      destroyNode(tmp);
    }
    m_head->nexts[0] = m_head;
    m_head->prevs[0] = m_head;
  }

  /*
   * @brief pick a random height for inserted skiplist entry
   */
  size_t
  pickRandomLevel() const
  {
    static boost::random::mt19937 gen;
    static boost::random::geometric_distribution<size_t> dist(Traits::getProbability());
    return std::min(dist(gen), Traits::getMaxLevels());
  }

protected:
  NodePointer m_head;
  std::allocator<Node> m_skiplistAllocator;
  std::allocator<T> m_dataAllocator;
  Compare m_compare;
  size_t m_size;
};


template<typename T, typename Compare, typename Traits>
typename SkipList<T, Compare, Traits>::const_iterator
SkipList<T, Compare, Traits>::lower_bound(const T& x) const
{
  size_t nLevels = m_head->nexts.size();
  NodePointer p = m_head;
  NodePointer q = p->nexts[nLevels - 1];
  for (int i = nLevels - 1; i >= 0; --i) {
    q = p->nexts[i];
    if (q != m_head) {
      while (m_compare(q->data, x)) {
        p = p->nexts[i];
        q = p->nexts[i];
        if (q == m_head) {
          break;
        }
      }
    }
  }
  return const_iterator(q);
}

template<typename T, typename Compare, typename Traits>
typename SkipList<T, Compare, Traits>::const_iterator
SkipList<T, Compare, Traits>::find(const T& x) const
{
  const_iterator it = this->lower_bound(x);
  if (it == this->end() || *it != x)
    return this->end();
  return it;
}

template<typename T, typename Compare, typename Traits>
std::pair<typename SkipList<T, Compare, Traits>::const_iterator, bool>
SkipList<T, Compare, Traits>::insert(const T& x)
{
  size_t nLevels = m_head->nexts.size();
  // 1. find insert position
  std::vector<NodePointer> insertPositions(nLevels);
  NodePointer p = m_head;
  NodePointer q = p->nexts[nLevels - 1];
  for (int i = nLevels - 1; i >= 0; --i) {
    q = p->nexts[i];
    if (q != m_head) {
      while (m_compare(q->data, x)) {
        p = p->nexts[i];
        q = p->nexts[i];
        if (q == m_head) {
          break;
        }
      }
    }
    insertPositions[i] = p;
  }
  // 2. whether q->data == x?
  if (q != m_head)
    if (!m_compare(q->data, x) && !m_compare(x, q->data)) {
      return std::pair<const_iterator, bool>(const_iterator(q), false);
    }
  // 3. construct new node;
  NodePointer newNode = createNode(x);
  // 4. pick random nLevels
  size_t newLevel = pickRandomLevel();
  // 5. insert the new node
  newNode->nexts.resize(newLevel + 1);
  newNode->prevs.resize(newLevel + 1);
  if (newLevel > nLevels - 1) {
    m_head->nexts.resize(newLevel + 1, m_head);
    m_head->prevs.resize(newLevel + 1, m_head);
    insertPositions.resize(newLevel + 1, m_head);
  }
  for (int i = 0; i <= newLevel; i++) {
    newNode->nexts[i] = insertPositions[i]->nexts[i];
    newNode->prevs[i] = insertPositions[i];
    insertPositions[i]->nexts[i] = newNode;
    newNode->nexts[i]->prevs[i] = newNode;
  }

  ++m_size;
  return std::pair<const_iterator, bool>(const_iterator(newNode), true);
}

template<typename T, typename Compare, typename Traits>
typename SkipList<T, Compare, Traits>::const_iterator
SkipList<T, Compare, Traits>::erase(typename SkipList<T, Compare, Traits>::const_iterator it)
{
  NodePointer eraseNode = it.node;
  if (!empty() && eraseNode != m_head) {
    NodePointer returnNode = eraseNode->nexts[0];
    size_t nLevels = eraseNode->nexts.size();
    for (int i = nLevels - 1; i >= 0; --i) {
      eraseNode->nexts[i]->prevs[i] = eraseNode->prevs[i];
      eraseNode->prevs[i]->nexts[i] = eraseNode->nexts[i];
      // clear empty nLevels
      if ((eraseNode->nexts[i] == eraseNode->prevs[i]) && i > 0) {
        m_head->nexts.pop_back();
        m_head->prevs.pop_back();
      }
    }
    destroyNode(eraseNode);
    --m_size;
    return const_iterator(returnNode);
  }
  else {
    return end();
  }
}

} // namespace repo

#endif // REPO_STORAGE_SKIPLIST_HPP
