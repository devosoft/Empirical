//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using IntPack

#include <iostream>
#include <string>
#include <vector>

#include "emp/meta/StringType.hpp"

int main()
{
  EMP_TEXT_TYPE(test_t, "This is a test!");
  std::cout << test_t::ToString() << '\n';


  std::cout << EMP_TEXT_HASH("Test Hash!") << '\n';
  std::cout << EMP_TEXT_HASH("Test Hash2!") << '\n';
  std::cout << EMP_TEXT_HASH("Test Hash3!") << '\n';
  std::cout << EMP_TEXT_HASH("Test Hash!") << '\n';
  std::cout << EMP_TEXT_HASH("Test Hash3!") << '\n';
}
