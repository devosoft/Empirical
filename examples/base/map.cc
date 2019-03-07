//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE

#include <algorithm>

#include "base/map.h"

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
}
