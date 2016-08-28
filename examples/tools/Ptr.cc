//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#define EMP_TRACK_MEM

#include "../../tools/Ptr.h"

int main()
{
  std::string test_str = "Test String!";
  std::cout << test_str << std::endl;

  emp::Ptr<std::string> test_ptr(test_str);
  std::cout << "From Ptr: " << *test_ptr << std::endl;
  std::cout << "  with size = " << test_ptr->size() << std::endl;

  {
    emp::Ptr<std::string> test_ptr2(test_str);
  }

  emp::Ptr<int> int_ptr;
  int_ptr.New(123456);
  std::cout << "*int_ptr = " << *int_ptr << std::endl;
  int_ptr.Delete();
}
