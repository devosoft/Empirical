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

  emp::vector<int> v = { 14, 13, 1, 2, 3, 4, 22, 5, 6, 7, 8, 9, 10, 12 };

  
  emp::Print(v);  std::cout << std::endl;

  std::cout << "Min index = " << emp::FindMinIndex(v) << std::endl;
  std::cout << "Max index = " << emp::FindMaxIndex(v) << std::endl;

  emp::Heapify(v);

  std::cout << "\nPost Heapify:\n";
  emp::Print(v);  std::cout << std::endl;

  int result = emp::HeapExtract(v);
  std::cout << "Max value = " << result << std::endl;

  std::cout << "\nPost Extract-Max:\n";
  emp::Print(v);  std::cout << std::endl;
  std::cout << "Min index = " << emp::FindMinIndex(v) << std::endl;
  std::cout << "Max index = " << emp::FindMaxIndex(v) << std::endl;
  
  emp::HeapInsert(v, 11);

  std::cout << "\nPost Insert(11):\n";
  emp::Print(v);  std::cout << std::endl;
  std::cout << "Min index = " << emp::FindMinIndex(v) << std::endl;
  std::cout << "Max index = " << emp::FindMaxIndex(v) << std::endl;
}
