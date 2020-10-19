//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "emp/base/array.hpp"

#define A_SIZE 50

// Function to print an std::array; will it work with emp::array?
template <size_t N>
void ArrayPrint(const std::array<int,N> & ar) {
  for (int x : ar) std::cout << x << " ";
  std::cout << std::endl;

  std::cout << ar << std::endl;
}

int main()
{
  emp::array<int, A_SIZE> test_array;

  for (size_t i = 0; i < A_SIZE; i++) {
    test_array[i] = (int) (i * i);
  }

  std::cout << "First array: " << std::endl;
  ArrayPrint<A_SIZE>(test_array);

  int sum = 0;
  for (size_t i = 0; i < A_SIZE; i++) {
    sum += test_array[i];
  }

  std::cout << "Sum of 0 through " << A_SIZE << " = " << sum << std::endl;

  emp::array<std::string, 5> array_lit = {{ "this", "is", "an", "array", "literal" }};
  for (const std::string & word : array_lit) {
    std::cout << word << " ";
  }
  std::cout << std::endl;
}
