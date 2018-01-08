//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ce_string

#include <iostream>

#include "base/array.h"
#include "constexpr/ce_string.h"
#include "tools/BitSet.h"

int main()
{
  constexpr emp::ce_string s = "abc";
  constexpr emp::ce_string s2 = "abc";
  constexpr emp::ce_string s3 = "abcdef";
  constexpr emp::ce_string s4 = "aba";
  emp::BitSet<s.size()> b1;
  emp::BitSet<(int) s[0]> b2;

  constexpr bool x1 = (s == s2);
  constexpr bool x2 = (s != s2);
  constexpr bool x3 = (s < s2);
  constexpr bool x4 = (s > s2);
  constexpr bool x5 = (s <= s2);
  constexpr bool x6 = (s >= s2);

  std::cout << x1 << x2 << x3 << x4 << x5 << x6 << std::endl;

  constexpr bool y1 = (s == s3);
  constexpr bool y2 = (s != s3);
  constexpr bool y3 = (s < s3);
  constexpr bool y4 = (s > s3);
  constexpr bool y5 = (s <= s3);
  constexpr bool y6 = (s >= s3);

  std::cout << y1 << y2 << y3 << y4 << y5 << y6 << std::endl;

  constexpr bool z1 = (s == s4);
  constexpr bool z2 = (s != s4);
  constexpr bool z3 = (s < s4);
  constexpr bool z4 = (s > s4);
  constexpr bool z5 = (s <= s4);
  constexpr bool z6 = (s >= s4);

  std::cout << z1 << z2 << z3 << z4 << z5 << z6 << std::endl;

  std::cout << "Test..."
            << "b2 size=" << b2.size()
            << std::endl;

  std::cout << "Test2..."
            << "new size=" << s.size()
            << std::endl;

  //  constexpr std::array<emp::ce_string, 3> test_array = {{ "abc", "def", "ghijkl" }};
  //  std::cout << test_array[1].ToString() << std::endl;
}
