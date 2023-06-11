/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file SmallVector.cpp
 *
 *  Adapted in part from the LLVM Project, under the Apache License v2.0 with
 *  LLVM Exceptions. See https://llvm.org/LICENSE.txt for license information.
 *  SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#include <list>
#include <stdarg.h>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/SmallVector.hpp"

TEST_CASE("Small vector, iterators", "[tools]") {
  emp::SmallVector<int, 4> v;
  REQUIRE(v.empty());
  REQUIRE(v.begin() == v.end());
  REQUIRE(v.rbegin() == v.rend());

  v.push_back(0);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  // Forward iterator
  size_t i = 0;
  for (emp::SmallVector<int, 4>::iterator it = v.begin(); it != v.end(); ++it) {
    REQUIRE(*it == v[i]);
    i++;
  }

  // Reverse iterator
  i = v.size()-1;
  for (emp::SmallVector<int, 4>::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) {
    REQUIRE(*it == v[i]);
    i--;
  }

  // Constant iterator
  const emp::SmallVector<int, 5> v2 = {4,3,2,1,0};
  i = 0;
  for (emp::SmallVector<int, 5>::const_iterator it = v2.begin(); it != v2.end(); ++it) {
    REQUIRE(*it == v2[i]);
    i++;
  }

  // Constant reverse iterator
  i = v2.size()-1;
  for (emp::SmallVector<int, 5>::const_reverse_iterator it = v2.rbegin(); it != v2.rend(); ++it) {
    REQUIRE(*it == v2[i]);
    i--;
  }

  // Data buffer pointers
  REQUIRE(*v.data() == 0);
  REQUIRE(*v2.data() == 4);

  // Iterator based insert
  emp::SmallVector<int, 10> v3 = {3};
  v3.emp::SmallVectorImpl<int>::insert(v3.begin(), 2);
  v3.emp::SmallVectorImpl<int>::insert(v3.end(), 1, 4);
  v3.emp::SmallVectorImpl<int>::insert(v3.end(), v3.begin(), v3.begin()+2);
  v3.emp::SmallVectorImpl<int>::insert(v3.end(), {6,7});
  int Elt = 8;
  v3.emp::SmallVectorImpl<int>::insert(v3.end(), std::move(Elt));
  REQUIRE(v3 == emp::SmallVector<int, 10>{2,3,4,2,3,6,7,8});
  REQUIRE(v3.size() == 8);

  v3.emp::SmallVectorImpl<int>::insert(std::next(v3.begin()), 99);
  REQUIRE(v3 == emp::SmallVector<int, 10>{2,99,3,4,2,3,6,7,8});
  REQUIRE(v3.size() == 9);

  v3.emp::SmallVectorImpl<int>::insert(std::prev(v3.end()), 3, 99);
  REQUIRE(v3 == emp::SmallVector<int, 10>{2,99,3,4,2,3,6,7,99,99,99,8});
  REQUIRE(v3.size() == 12);

  v3.emp::SmallVectorImpl<int>::insert(
    std::prev(v3.end()), v3.begin(), v3.begin()+2
  );
  REQUIRE(v3 == emp::SmallVector<int, 10>{2,99,3,4,2,3,6,7,99,99,99,2,99,8});
  REQUIRE(v3.size() == 14);

  v3.emp::SmallVectorImpl<int>::insert(
    std::next(v3.begin()), v3.begin(), v3.begin()+1
  );
  REQUIRE(v3 == emp::SmallVector<int, 10>{2,2,99,3,4,2,3,6,7,99,99,99,2,99,8});
  REQUIRE(v3.size() == 15);

}

TEST_CASE("Small vector, methods", "[tools]") {
    // Front and back pointers
    emp::SmallVector<int, 4> v = {0,1,2,3};
    REQUIRE(v.front() == v[0]);
    REQUIRE(v.back() == v[v.size()-1]);

    const emp::SmallVector<int, 5> v2 = {4,3,2,1,0};
    REQUIRE(v2.front() == v2[0]);
    REQUIRE(v2.back() == v2[v2.size()-1]);

    // Size and capacity methods
    REQUIRE(v.size_in_bytes() < v2.size_in_bytes());
    REQUIRE(v.max_size() > 0);
    REQUIRE(v.capacity_in_bytes() <= capacity_in_bytes(v2));

    // std::swap smallvector implementation
    emp::SmallVector<int, 4> v3 = v;
    emp::SmallVector<int, 4> v4 = {8,7,6,5};
    std::swap(v3, v4);
    REQUIRE(v3 != v);
    REQUIRE(v4 < v3);

    // Reserve
    v3.reserve(10);
    std::swap(v3, v4);
    REQUIRE(v3.size() == 4);
    std::swap(v3, v4);

    v4.reserve(5);
    REQUIRE(v3.capacity() == 10);
    std::swap(v3, v4);
    REQUIRE(v3.size() == 4);
    std::swap(v3, v4);

    // Append and Assign more than capacity
    const int i = 7;
    v3.append(7,i);
    REQUIRE(v3.capacity() > 10);
    v4.append(2,i);
    REQUIRE(v4.capacity() > 5);
    std::swap(v3, v4);
    REQUIRE(v4.capacity() > 10);

    emp::SmallVector<int, 4> v8 = {8,7};
    emp::SmallVector<int, 4> v9 = {1,2,6,5};
    std::swap(v8, v9);
    REQUIRE(v8.size() == 4);
    REQUIRE(v9.size() == 2);
    std::swap(v8, v9);
    REQUIRE(v9.size() == 4);
    REQUIRE(v8.size() == 2);

    emp::SmallVector<int, 4> empty{};
    v8 = empty;
    REQUIRE( v8.empty() );

    v9 = emp::SmallVector<int, 4>{};
    REQUIRE( v9.empty() );

    v9 = emp::SmallVector<int, 4>{1, 2};
    REQUIRE( v9.size() == 2 );
    v9 = emp::SmallVector<int, 4>{1};
    REQUIRE( v9.size() == 1 );

}

TEST_CASE("Small vector, constructors", "[tools]") {
    // Adapted from https://en.cppreference.com/w/cpp/container/vector/vector
    const emp::SmallVector<std::string, 5> words0 = {"the", "frogurt", "is", "also", "cursed"};
    emp::SmallVector<std::string, 5> words1 {"the", "frogurt", "is", "also", "cursed"};

    // words2 == words1
    emp::SmallVector<std::string, 5> words2(words1.begin(), words1.end());

    // words3 == words1
    emp::SmallVector<std::string, 5> words3(words1);

    // words4 is {"Mo", "Mo", "Mo", "Mo", "Mo"}
    emp::SmallVector<std::string, 5> words4(5, "Mo");

    REQUIRE(words0 == words1);
    REQUIRE(words1 == words2);
    REQUIRE(words2 == words3);
    REQUIRE(words4[4] == "Mo");

    // Move Constructors
    emp::SmallVector<std::string, 5> words5((emp::SmallVector<std::string, 5>(words1)));
    const emp::SmallVector<std::string, 5> words6((emp::SmallVector<std::string, 5>(words1)));
    REQUIRE(words1 == words5);
    REQUIRE(words1 == words6);

    //Operator=
    emp::SmallVector<std::string, 5> words7;
    words7 = words1;
    const emp::SmallVector<std::string, 5> words8 = words6;
    REQUIRE(words1 == words7);
    REQUIRE(words6 == words8);

    words1.resize(10, "a");
    words7 = words1;
    REQUIRE(words1 == words7);
    words1.resize(5);
    words7 = words1;
    REQUIRE(words1 == words7);

}

TEST_CASE("Small vector, methods2", "[tools]") {
  // Adapted from vector.cc
  emp::SmallVector<std::string, 3> vec = {"a", "b", "c"};
  std::string sum;
  for(auto it=vec.begin(); it!=vec.end(); it++){
    sum += *it;
  }
  REQUIRE(sum == "abc");

  // resize
  emp::SmallVector<bool, 0> bvec;
  bvec.resize(1);
  REQUIRE(bvec.size() == 1);
  bvec[0] = true;
  REQUIRE(bvec[0] == true);
  bvec.resize(5,false);
  REQUIRE(bvec[1] == false);
  bvec.resize(4);
  REQUIRE(bvec.size() == 4);
  bvec.resize(3, true);
  REQUIRE(bvec.size() == 3);
  bvec.resize(100, true);
  REQUIRE(bvec.size() == 100);
  bvec.resize(1000, true);
  REQUIRE(bvec.size() == 1000);

  // pop_back and pop_back_val
  emp::SmallVector<bool, 4> bvec2 = { true, false, true, false };
  REQUIRE(bvec2.size() == 4);
  bvec2.pop_back();
  REQUIRE(bvec2.size() == 3);
  REQUIRE(bvec2.pop_back_val() == true);

  // Assign with iterators
  emp::SmallVector<bool, 4> bvec3 = { true, false, true, false };
  emp::SmallVector<bool, 6> bvec4 = { true, false, true, false, true, false};
  bvec3.assign(bvec4.begin(),bvec4.end());
  REQUIRE(bvec3.size() == 6);
  REQUIRE(bvec3 == bvec4);

  // Erase
  emp::SmallVector<int,10> myvector;
  for (int i=1; i<=10; i++) myvector.push_back(i);

  // erase the 6th element
  myvector.erase (myvector.begin()+5);
  REQUIRE(myvector.size() == 9);

  // erase the first 3 elements:
  myvector.erase (myvector.begin(),myvector.begin()+3);
  REQUIRE(myvector.size() == 6);

  // Emplace Back
  myvector.emplace_back(100);
  REQUIRE(myvector[6] == 100);

}
