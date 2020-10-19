/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  flex_function.h
 *  @brief A collection of broadly-useful functions (that don't fit elsewhere)
 *  @note Status: BETA (though new functions are added frequently)
 */


#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <functional>
#include <iostream>
#include <type_traits>
#include <sstream>

#include "../base/assert.hpp"
#include "../base/vector.hpp"

#include "../math/constants.hpp"
#include "../math/math.hpp"

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

  /// Toggle an input bool.
  inline bool Toggle(bool & in_bool) { return (in_bool = !in_bool); }

  /// Combine bools to AND them all together.
  inline constexpr bool AllTrue() { return true; }
  template <typename... Ts>
  inline bool AllTrue(bool result, Ts... OTHER) {
    return result && AllTrue(OTHER...);
  }

  /// Combine bools to OR them all together.
  inline constexpr bool AnyTrue() { return false; }
  template <typename... Ts>
  inline bool AnyTrue(bool result, Ts... OTHER) {
    return result || AnyTrue(OTHER...);
  }

  /// Build a vector with a range of values from min to max at the provided step size.
  template <typename T>
  static inline emp::vector<T> BuildRange(T min, T max, T step=1) {
    emp_assert(max > min);
    size_t vsize = (size_t) ((max-min) / step) + 1;
    emp::vector<T> out_v(vsize);
    size_t pos = 0;
    for (T i = min; i < max; i += step) {
      out_v[pos++] = i;
    }
    return out_v;
  }


  /// Determine the size of a built-in array.
  template <typename T, size_t N>
  constexpr size_t GetSize(T (&)[N]) { return N; }

  /// A function that will always return a unique value (and trip an assert if it can't...)
  static size_t UniqueVal() {
    static size_t val = 0;
    emp_assert(val < MaxValue<size_t>() && "Ran out of unique values in size_t!");
    return val++;
  }

  /// A function that will always return a unique stringname (using UniqVal) with provided
  /// prefix and postfix.)
  static inline std::string UniqueName(const std::string & prefix="",
                                       const std::string & postfix="") {
    std::stringstream ss;
    ss << prefix << UniqueVal() << postfix;
    return ss.str();
  }
}

#endif
