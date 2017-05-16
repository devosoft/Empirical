//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A collection of broadly-useful functions.

#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <functional>
#include <iostream>
#include <type_traits>
#include <sstream>

#include "../base/assert.h"
#include "../base/vector.h"

#include "const.h"
#include "math.h"

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

  static inline double TimeFun(std::function<void()> test_fun) {
    std::clock_t start_time = std::clock();
    test_fun();
    std::clock_t tot_time = std::clock() - start_time;
    return 1000.0 * ((double) tot_time) / (double) CLOCKS_PER_SEC;
  }

  // Toggle an input bool.
  inline bool Toggle(bool & in_bool) { return (in_bool = !in_bool); }


  template <typename T>
  static inline emp::vector<T> BuildRange(T min, T max, T step=1) {
    size_t size = (size_t) ((max-min) / step);
    emp_assert(size >= 0);
    emp::vector<T> out_v((size_t) size);
    size_t pos = 0;
    for (T i = min; i < max; i += step) {
      out_v[pos++] = i;
    }
    return out_v;
  }


  // Build a function to determine the size of a built-in array.
  template <typename T, size_t N>
  constexpr size_t GetSize(T (&)[N]) { return N; }

  // Build a function that will always return a unique value (and trip an assert if it can't...)
  static size_t UniqueVal() {
    static size_t val = 0;
    emp_assert(val < MaxValue<size_t>() && "Ran out of unique values in size_t!");
    return val++;
  }

  static inline std::string UniqueName(const std::string & prefix="",
                                       const std::string & postfix="") {
    std::stringstream ss;
    ss << prefix << UniqueVal() << postfix;
    return ss.str();
  }
}

#endif
