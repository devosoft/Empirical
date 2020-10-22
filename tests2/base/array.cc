/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 * 
 *  Unit Tests for emp::array template.
 **/

#include "../unit_tests.h"

#include "base/array.h"

#include <sstream>
#include <iostream>

void emp_test_main()
{
  //TEST_CASE("Test array", "[base]")
  const size_t SIZE = 10;
  emp::array<int, SIZE> arr = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  EMP_TEST_VALUE(arr[5], 5);

  emp::array<int, SIZE> arr2(arr);
  EMP_TEST_VALUE(arr, arr2);
  std::array<int, SIZE> s_arr = (std::array<int,SIZE>) arr2;
  EMP_TEST_VALUE(s_arr[8], arr2[8]);
  EMP_TEST_VALUE(arr.back(), 9);
  EMP_TEST_VALUE(arr.front(), 0);

  int three = arr[3];
  EMP_TEST_VALUE(three, 3);

  int one = arr[1];
  EMP_TEST_VALUE(one, 1);

  std::stringstream ss;
  ss << arr;
  EMP_TEST_VALUE(ss.str(), "0 1 2 3 4 5 6 7 8 9 ");

  emp::array<int, SIZE> arr3;
  ss >> arr3;
  EMP_TEST_VALUE(arr3[2], 2);
  ss.str(std::string());

  //TEST_CASE("Another test array", "[base]")

  constexpr int A_SIZE = 50;
  emp::array<int, A_SIZE> test_array;

  for (size_t i = 0; i < A_SIZE; i++) {
    test_array[i] = (int) (i * i);
  }

  int sum = 0;
  for (size_t i = 0; i < A_SIZE; i++) {
    sum += test_array[i];
  }

  EMP_TEST_VALUE(sum, 40425);
}
