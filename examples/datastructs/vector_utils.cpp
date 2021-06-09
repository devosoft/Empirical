//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017-2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using vector_utils.h

#include <iostream>

#include "emp/datastructs/vector_utils.hpp"

int main()
{
  emp::vector<std::string> v1 = { "a", "b", "cde" };
  emp::vector<std::string> v2 = { "f", "g", "hij" };
  emp::vector<std::string> v3 = { "klm", "n", "op" };
  emp::vector<std::string> v4 = { "qrstuv", "wxy", "z" };

  auto all = emp::Concat(v1, v2, v3, v4);

  std::cout << "Words: ";
  for (const auto & w : all) {
    std::cout << w << " ";
  }
  std::cout << '\n';

  emp::vector<int> v = { 14, 13, 1, 2, 3, 4, 22, 5, 6, 7, 8, 9, 10, 12 };


  emp::Print(v);  std::cout << '\n';

  std::cout << "Min index = " << emp::FindMinIndex(v) << '\n';
  std::cout << "Max index = " << emp::FindMaxIndex(v) << '\n';

  emp::Heapify(v);

  std::cout << "\nPost Heapify:\n";
  emp::Print(v);  std::cout << '\n';

  int result = emp::HeapExtract(v);
  std::cout << "Max value = " << result << '\n';

  std::cout << "\nPost Extract-Max:\n";
  emp::Print(v);  std::cout << '\n';
  std::cout << "Min index = " << emp::FindMinIndex(v) << '\n';
  std::cout << "Max index = " << emp::FindMaxIndex(v) << '\n';

  emp::HeapInsert(v, 11);

  std::cout << "\nPost Insert(11):\n";
  emp::Print(v);  std::cout << '\n';
  std::cout << "Min index = " << emp::FindMinIndex(v) << '\n';
  std::cout << "Max index = " << emp::FindMaxIndex(v) << '\n';

  std::cout << "\nNow trying out FindEval for values from 7 to 9:\n";
  emp::Print(v);  std::cout << '\n';
  int found_pos = emp::FindEval(v, [](int x){ return (x>=7 && x <=9); });
  std::cout << "First position found: " << found_pos << '\n';
  found_pos = emp::FindEval(v, [](int x){ return (x>=7 && x <=9); }, found_pos+1);
  std::cout << "Next position found: " << found_pos << '\n';
  found_pos = emp::FindEval(v, [](int x){ return (x>=7 && x <=9); }, found_pos+1);
  std::cout << "Next position found: " << found_pos << '\n';
  found_pos = emp::FindEval(v, [](int x){ return (x>=7 && x <=9); }, found_pos+1);
  std::cout << "Next position found: " << found_pos << '\n';

}
