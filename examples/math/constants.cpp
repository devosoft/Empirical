//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::Random

#include <iostream>
#include "emp/math/constants.hpp"

#define TEST_PRINT(T) std::cout << "MaxValue<" << #T << ">() = " << (uint64_t) emp::MaxValue<T>() << std::endl;
#define TEST_PRINT2(T) std::cout << "MaxValue<" << #T << ">() = " << emp::MaxValue<T>() << std::endl;

int main()
{
  TEST_PRINT(int8_t);
  TEST_PRINT(uint8_t);
  TEST_PRINT(int16_t);
  TEST_PRINT(uint16_t);
  TEST_PRINT(int32_t);
  TEST_PRINT(uint32_t);
  TEST_PRINT(int64_t);
  TEST_PRINT(uint64_t);

  TEST_PRINT2(float);
  TEST_PRINT2(double);
}
