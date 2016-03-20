//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

#include "../../tools/array.h"

#define A_SIZE 50

int main()
{
  emp::array<int, A_SIZE> test_array;

  for (int i = 0; i < A_SIZE; i++) {
    test_array[i] = i * i;
  }

  int sum = 0;
  for (int i = 0; i < A_SIZE; i++) {
    sum += test_array[i];
  }

  std::cout << sum << std::endl;
}
