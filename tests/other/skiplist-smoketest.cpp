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

#include "skiplist-list.hpp" // This skiplist is updated by weiqi.
                              // The internal structure of skiplist node is std::list

#include "skiplist-vector.hpp" // This skiplist is revised is revised based on the version above
                               // The internal structure of skiplist node is std::vector

#include "skiplist-prev.hpp" // This skiplist is that of previous commit

#include <iostream>
#include <set>

using namespace ndn::time;

namespace repo {
namespace tests {

void
testSkipList()
{
  typedef update1::SkipList<int, std::greater<int> > IntGtContainer;
  IntGtContainer sl;
  steady_clock::TimePoint start = steady_clock::now();
  for (int i = 0; i < 100000; ++i) {
    sl.insert(i);
  }
  milliseconds duration = duration_cast<milliseconds>(steady_clock::now() - start);
  start = steady_clock::now();
  std::cout << "SkipList-list insert 100000 integers cost " << duration.count() << "ms" << std::endl;
  for (int i = 0; i< 100000; ++i) {
    sl.lower_bound(i);
  }
  duration = duration_cast<milliseconds>(steady_clock::now() - start);
  std::cout << "SkipList-list lower_bound 100000 integers cost " << duration.count() << "ms" << std::endl;
}

void
testSkipVector()
{
  typedef update2::SkipList<int, std::greater<int> > IntGtContainer;
  IntGtContainer container;
  steady_clock::TimePoint start = steady_clock::now();
  for (int i = 0; i < 100000; ++i) {
    container.insert(i);
  }
  milliseconds duration = duration_cast<milliseconds>(steady_clock::now() - start);
  start = steady_clock::now();
  std::cout << "Skiplist-vector insert 100000 integers cost " << duration.count() << "ms" << std::endl;
  for (int i = 0; i< 100000; ++i) {
    container.lower_bound(i);
  }
  duration = duration_cast<milliseconds>(steady_clock::now() - start);
  std::cout << "Skiplist-vector lower_bound 100000 integers cost " << duration.count() << "ms" << std::endl;
}

void
testSkipPrev()
{
  typedef prev::SkipList<int, std::greater<int> > IntGtContainer;
  IntGtContainer container;
  steady_clock::TimePoint start = steady_clock::now();
  for (int i = 0; i < 100000; ++i) {
    container.insert(i);
  }
  milliseconds duration = duration_cast<milliseconds>(steady_clock::now() - start);
  start = steady_clock::now();
  std::cout << "Skiplist-prev insert 100000 integers cost " << duration.count() << "ms" << std::endl;
  for (int i = 0; i< 100000; ++i) {
    container.lower_bound(i);
  }
  duration = duration_cast<milliseconds>(steady_clock::now() - start);
  std::cout << "Skiplist-prev lower_bound 100000 integers cost " << duration.count() << "ms" << std::endl;
}

void
testSet()
{
  typedef std::set<int, std::greater<int> > IntGtContainer;
  IntGtContainer container;
  steady_clock::TimePoint start = steady_clock::now();
  for (int i = 0; i < 100000; ++i) {
    container.insert(i);
  }
  milliseconds duration = duration_cast<milliseconds>(steady_clock::now() - start);
  start = steady_clock::now();
  std::cout << "Set insert 100000 integers cost " << duration.count() << "ms" << std::endl;
  for (int i = 0; i< 100000; ++i) {
    container.lower_bound(i);
  }
  duration = duration_cast<milliseconds>(steady_clock::now() - start);
  std::cout << "Set lower_bound 100000 integers cost " << duration.count() << "ms" << std::endl;
}

void runTestCases() {
  testSkipList();
  testSkipVector();
  testSkipPrev();
  testSet();
}

} // namespace tests
} // namespace repo

int
main(int argc, char** argv)
{
  repo::tests::runTestCases();

  return 0;
}