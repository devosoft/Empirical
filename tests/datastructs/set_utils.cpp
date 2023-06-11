/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file set_utils.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/set_utils.hpp"

TEST_CASE("Test set utils", "[datastructs]") {
  std::set<int> s1;
  std::set<int> s2;
  std::set<int> comp_set;
  emp::vector<int> v1;
  emp::vector<int> v2;

  s1.insert(1);
  s1.insert(2);
  s2.insert(2);
  s2.insert(3);
  v1.push_back(1);
  v1.push_back(3);
  v2.push_back(4);
  v2.push_back(1);

  REQUIRE(emp::Has(s1, 1));
  REQUIRE(!emp::Has(s1, 3));

  comp_set.insert(1);
  REQUIRE(emp::difference(s1, s2) == comp_set);
  comp_set.clear();
  comp_set.insert(3);
  REQUIRE(emp::difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::intersection(s1, s2) == comp_set);
  REQUIRE(emp::intersection(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  REQUIRE(emp::difference(s1, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(3);
  REQUIRE(emp::difference(v1, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  REQUIRE(emp::intersection(s1, v1) == comp_set);
  REQUIRE(emp::intersection(v1, s1) == comp_set);
  REQUIRE(emp::intersection(v2, v1) == comp_set);
  REQUIRE(emp::intersection(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  REQUIRE(emp::difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::set_union(s1, s2) == comp_set);
  REQUIRE(emp::set_union(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(2);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, s2) == comp_set);
  REQUIRE(emp::set_union(s2, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  comp_set.insert(4);
  REQUIRE(emp::set_union(v2, v1) == comp_set);
  REQUIRE(emp::set_union(v1, v2) == comp_set);
  comp_set.clear();
  comp_set.insert(1);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(s1, s2) == comp_set);
  REQUIRE(emp::symmetric_difference(s2, s1) == comp_set);
  comp_set.clear();
  comp_set.insert(4);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, v2) == comp_set);
  REQUIRE(emp::symmetric_difference(v2, v1) == comp_set);
  comp_set.clear();
  comp_set.insert(2);
  comp_set.insert(3);
  REQUIRE(emp::symmetric_difference(v1, s1) == comp_set);
  REQUIRE(emp::symmetric_difference(s1, v1) == comp_set);

  std::multiset<int> m_set;
  m_set.insert(4);
  m_set.insert(3);
  REQUIRE(emp::Has(m_set, 3));
  REQUIRE(!emp::Has(m_set, 5));

  std::unordered_multiset<int> um_set;
  um_set.insert(6);
  um_set.insert(3);
  REQUIRE(emp::Has(um_set, 3));
  REQUIRE(!emp::Has(um_set, 7));

  std::unordered_set<int> u_set;
  u_set.insert(9);
  u_set.insert(7);
  REQUIRE(emp::Has(u_set, 7));
  REQUIRE(!emp::Has(u_set, 4));

}
