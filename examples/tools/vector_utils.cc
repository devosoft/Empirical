//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using vector_utils.h

#include <iostream>

#include "tools/vector_utils.h"

int main()
{

  emp::vector<int> v = { 1, 2, 3, 4, 22, 5, 6, 7, 8, 9, 10, 12 };
  
  emp::Print(v);  std::cout << std::endl;

  emp::Heapify(v);

  emp::Print(v);  std::cout << std::endl;

  int result = emp::HeapExtract(v);
  std::cout << "Max = " << result << std::endl;

  emp::Print(v);  std::cout << std::endl;
  
  emp::HeapInsert(v, 11);

  emp::Print(v);  std::cout << std::endl;
}
