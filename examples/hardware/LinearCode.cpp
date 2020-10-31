/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaGP.h
 *  @brief This is example code for using LinearCode.h
 */

#include <iostream>

#include "emp/hardware/LinearCode.hpp"
#include "emp/math/Random.hpp"

int main()
{
  std::cout << "Test." << std::endl;

  emp::LinearCode<> test_code;
  test_code.push_back(emp::Instruction<3>(3, {1,2,3}));
  test_code.AddInst(1);
  test_code.AddInst(2, {1});
  test_code.AddInst(3, {1,2,3});

  std::cout << "Test code size = " << test_code.size() << std::endl;
}
