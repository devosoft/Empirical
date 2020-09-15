#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/set_utils.h"

#include <sstream>
#include <iostream>



TEST_CASE("Test set utils", "[tools]") {
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

}
