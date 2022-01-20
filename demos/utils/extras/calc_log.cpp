/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file calc_log.cpp
 */

#include <cmath>
#include <iostream>

int main()
{
  const double steps = 1024.0;
  for (double i = 0.0; i < steps; i += 1.0) {
    double val = 1.0 + i/steps;
    // std::cout << i << " " << steps << " " << val << " : " << std::log2(val) << std::endl;
    std::cout << std::log2(val) << ", ";
  }
  std::cout << std::endl;
}
