//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <algorithm>

#include "emp/base/unordered_map.hpp"

int main()
{
  emp::unordered_map<int, std::string> test_map;

  test_map[3] = "Three";
  test_map[0] = "Zero";
  test_map[1000] = "One Thousand";

  std::cout << "3 = " << test_map[3] << '\n' << '\n';

  test_map[-10] = "Minus Ten";
  test_map[1000000001] = "One Billion One";

  for (auto [key, value] : test_map) {
    std::cout << "  key = " << key
 	      << "  value = " << value
 	      << '\n';
  }


  emp::unordered_map<int, std::string> test_map_copy(test_map);
  std::cout << "\nCopy (by constructor):\n";
  for (auto [key, value] : test_map_copy) {
    std::cout << "  key = " << key
 	      << "  value = " << value
 	      << '\n';
  }

  test_map[67] = "Sixty Seven (New!!)";

  test_map_copy = test_map;
  std::cout << "\nCopy2 (by operator):\n";
  for (auto [key, value] : test_map_copy) {
    std::cout << "  key = " << key
 	      << "  value = " << value
 	      << '\n';
  }




  emp::unordered_map<std::string, int> test_map2;

  test_map2["one"] = 1;
  test_map2["two"] = 2;
  test_map2["three"] = 3;
  test_map2["counter"] = 0;

  std::cout << test_map2["one"] << " + "
	    << test_map2["two"] << " = "
	    << test_map2["three"] << '\n';

  test_map2["counter"] = test_map2["one"] + test_map2["two"];
  test_map2["counter"] += test_map2["three"];

  std::cout << (2 * test_map2["counter"] * 2) << " = " << 24
	    << '\n';


}
