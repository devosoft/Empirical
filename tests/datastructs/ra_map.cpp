/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/ra_map.hpp"

TEST_CASE("Test ra_set", "[datastructs]")
{
  emp::ra_map<std::string, double> test_map;

  CHECK(test_map.size() == 0);
  CHECK(test_map.empty() == true);

  test_map["pi"] = 3.14159265358979; // Try inserting a value

  CHECK(test_map.size() == 1);
  CHECK(test_map.empty() == false);

  test_map["ONE"] = 1.0;  // Try inserting more values
  test_map["TWO"] = 2.0;

  CHECK(test_map.size() == 3);

  test_map["pi"] = 3.0;   // Try changing a value already in the map.

  CHECK(test_map.size() == 3);

  test_map["half"] = 0.5;

  CHECK(test_map.size() == 4);

  // Try a range-based for loop.
  double total = 0.0;
  for (auto [name, x] : test_map) { total += x; }

  CHECK(total == 6.5);

  // Try copy constructor.
  emp::ra_map<std::string, double> test_map2(test_map);

  CHECK(test_map2.size() == 4);

  // Try removing a value from the new map only.
  test_map2.erase("pi");

  CHECK(test_map2.size() == 3);
  CHECK(test_map.size() == 4);  // Old map should remain unchanged.
}
