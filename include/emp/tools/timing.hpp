/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  timing.hpp
 *  @brief A collection of tools to help measure timing of code.
 *  @note Status: BETA
 */


#ifndef EMP_TIMING_H
#define EMP_TIMING_H

#include <ctime>
#include <functional>
#include <iostream>
#include <type_traits>

#include "../base/assert.hpp"

#include "../math/constants.hpp"

/// A simple macro to time how long it takes for a function to complete.
#define EMP_FUNCTION_TIMER(TEST_FUN) {                                       \
    std::clock_t emp_start_time = std::clock();                              \
    auto emp_result = TEST_FUN;                                              \
    std::clock_t emp_tot_time = std::clock() - emp_start_time;               \
    std::cout << "Time: "                                                    \
              << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC  \
              << " ms" << std::endl;                                         \
    std::cout << "Result: " << emp_result << std::endl;                      \
  }

namespace emp {

  /// A function timer that takes a functor an identifies how long it takes to complete when run.
  static inline double TimeFun(std::function<void()> test_fun) {
    std::clock_t start_time = std::clock();
    test_fun();
    std::clock_t tot_time = std::clock() - start_time;
    return 1000.0 * ((double) tot_time) / (double) CLOCKS_PER_SEC;
  }

}

#endif
