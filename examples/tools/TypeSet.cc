//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using TypeSet

#include <iostream>
#include <string>

#include "../../tools/TypeSet.h"

int Sum4(int a, int b, int c, int d) { return a+b+c+d; }

int main()
{
  std::cout << "Testing!" << std::endl;

  using test_t = emp::TypeSet<int, std::string, float, bool, double>;
  std::cout << "Num types = " << test_t::GetSize() << std::endl;
  std::cout << "float pos = " << test_t::GetID<float>() << std::endl;

  std::cout << "Num types (with add) = " << test_t::add_t<long long>::GetSize() << std::endl;
  std::cout << "float pos (with pop) = " << test_t::pop_t::GetID<float>() << std::endl;

  using test2_t = emp::TypeSet<int, int, int, int>;
  using test3_t = emp::TypeSet<uint64_t>;

  std::cout << std::endl;
  std::cout << "Num test_t types:  " << test_t::GetSize() << std::endl;
  std::cout << "Num test2_t types: " << test2_t::GetSize() << std::endl;
  std::cout << "Num test3_t types: " << test3_t::GetSize() << std::endl;

  // Make sure we can use a TypeSet to properly set a function type.
  using fun_t = test2_t::to_function_t<int>;
  std::function< fun_t > fun(Sum4);
  std::cout << "Sum4(1,2,3,4) = " << fun(1,2,3,4) << std::endl;

  using test4_t = test2_t::crop_t<2>;
  std::cout << "After crop_t<2>, size of type = " << test4_t::GetSize() << std::endl;
  std::cout << "After merge of t1 and t2, size = "
	    << test_t::merge_t<test2_t>::GetSize() << std::endl;
}
