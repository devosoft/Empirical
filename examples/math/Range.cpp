//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using math functions.

#include <iostream>

#include "emp/math/Range.hpp"
#include "emp/tools/string_utils.hpp"

int main()
{
  auto range = emp::MakeRange(20,30);
  std::cout << "Lower = " << range.GetLower() << '\n';
  std::cout << "Upper = " << range.GetUpper() << '\n';

  for (int i = 10; i < 40; i += 5) {
    std::cout << "Value " << i << " valid = " << range.Valid(i) << '\n';
  }

  for (size_t s = 4; s <= 8; s++) {
    auto spread = range.Spread(s);
    std::cout << "Spread(" << s << ") = " << emp::to_string(spread) << '\n';
  }
}
