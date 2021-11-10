/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020
 *
 *  @file map.cpp
 */

#include <algorithm>

#include "emp/base/map.hpp"

int main()
{
  emp::map<int, std::string> test_map;

  test_map[3] = "Three";
  test_map[0] = "Zero";
  test_map[1000] = "One Thousand";

  std::cout << "3 = " << test_map[3] << std::endl << std::endl;

  test_map[-10] = "Minus Ten";
  test_map[1000000001] = "One Billion One";

  for (auto [key, value] : test_map) {
    std::cout << "  key = " << key
         << "  value = " << value
         << std::endl;
  }


  emp::map<int, std::string> test_map_copy(test_map);
  std::cout << "\nCopy (by constructor):\n";
  for (auto [key, value] : test_map_copy) {
    std::cout << "  key = " << key
         << "  value = " << value
         << std::endl;
  }

  test_map[67] = "Sixty Seven (New!!)";

  test_map_copy = test_map;
  std::cout << "\nCopy2 (by operator):\n";
  for (auto [key, value] : test_map_copy) {
    std::cout << "  key = " << key
         << "  value = " << value
         << std::endl;
  }




  emp::map<std::string, int> test_map2;

  test_map2["one"] = 1;
  test_map2["two"] = 2;
  test_map2["three"] = 3;
  test_map2["counter"] = 0;

  std::cout << test_map2["one"] << " + "
      << test_map2["two"] << " = "
      << test_map2["three"] << std::endl;

  test_map2["counter"] = test_map2["one"] + test_map2["two"];
  test_map2["counter"] += test_map2["three"];

  std::cout << (2 * test_map2["counter"] * 2) << " = " << 24
      << std::endl;


}
