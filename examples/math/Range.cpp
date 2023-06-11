/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file Range.cpp
 */

#include <iostream>

#include "emp/math/Range.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  auto range = emp::MakeRange(20,30);
  std::cout << "Lower = " << range.GetLower() << std::endl;
  std::cout << "Upper = " << range.GetUpper() << std::endl;

  for (int i = 10; i < 40; i += 5) {
    std::cout << "Value " << i << " valid = " << range.Valid(i) << std::endl;
  }

  for (size_t s = 4; s <= 8; s++) {
    auto spread = range.Spread(s);
    std::cout << "Spread(" << s << ") = " << emp::to_string(spread) << std::endl;
  }
}
