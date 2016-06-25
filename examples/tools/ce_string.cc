//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::ce_string

#include <iostream>

#include "../../tools/ce_string.h"
#include "../../tools/BitSet.h"


int main()
{
  constexpr emp::ce_string s("abc");
  emp::BitSet<s.size()> b1;
  emp::BitSet<(int) s[0]> b2;


  std::cout << "Test..."
            << "b2 size=" << b2.size()
            << std::endl;
}
