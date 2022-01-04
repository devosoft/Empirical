/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file vector.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/vector.hpp"

TEST_CASE("Test vector", "[base]")
{
  emp::vector<std::string> vec = {"a", "b", "c"};
  std::string sum;
  for(auto it=vec.cbegin(); it!=vec.cend(); it++){
    sum += *it;
  }
  REQUIRE(sum == "abc");

  /// This throws an error...
  // because it's a reverse iterator?
  //sum += *vec.rend();
  //REQUIRE(sum == "abca");

  auto itr = vec.emplace(vec.begin()+1, "1");
  REQUIRE(*itr == "1");
  REQUIRE(*(itr-1) == "a");

  emp::vector<bool> bvec;
  bvec.resize(1);
  REQUIRE(bvec.size() == 1);
  bvec[0] = true;
  REQUIRE(bvec[0] == true);
  bvec.resize(5,false);
  REQUIRE(bvec[1] == false);

  /// Can't seem to get front() and back() to work for bool vector
  /// throws error on this line in vector.h "size_t pos = (size_t) (*this - v_ptr->begin());"
  //auto b = bvec.front();

  emp::vector<bool> bvec2 = { true, false, true, false };
  REQUIRE(bvec2.size() == 4);
  bvec2.pop_back();
  REQUIRE(bvec2.size() == 3);

#ifndef NDEBUG
  std::stringstream ss;
  ss << bvec2;
  REQUIRE(ss.str() == "1 0 1 ");
  ss.str(std::string());
#endif

  emp::vector<bool> bvec3(3);
  /// Can't do this
  /// "error: invalid initialization of non-const reference of type 'bool&' from an rvalue of type 'bool'"
  //ss >> bvec3;
}

TEST_CASE("Another test vector", "[base]")
{
  emp::vector<int> v(20);

  for (size_t i = 0; i < 20; i++) {
    v[i] = (int) (i * i);
  }

  int total = 0;
  for (int i : v) total += i;

  REQUIRE(total == 2470);

  // Examine vector<bool> specialization.
  emp::vector<bool> vb(1000,false);
  for (size_t i = 0; i < vb.size(); i++) {
    if (i%3==0 || i%5 == 0) vb[i] = true;
  }
  size_t count = 0;
  const auto vb2 = vb;
  for (size_t i = 0; i < vb.size(); i++) {
    if (vb2[i]) count++;
  }

  REQUIRE(count == 467);
}
