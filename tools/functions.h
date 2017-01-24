//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A collection of broadly-useful functions.

#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <sstream>
#include <vector>

#include "assert.h"
#include "const.h"

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

  /// % is actually remainder; this is a proper modulus command that handles negative #'s correctly
  inline constexpr int Mod(int in_val, int mod_val) {
    return (in_val < 0) ? (in_val % mod_val + mod_val) : (in_val % mod_val);
  }

  // A fast (O(log p)) integer-power command.
  static constexpr int Pow(int base, int p) {
    return (p <= 0) ? 1 : base * Pow(base, p-1);
  }


  // Run both min and max on a value to put it into a desired range.
  template <typename TYPE> constexpr TYPE ToRange(const TYPE & value, const TYPE & in_min, const TYPE & in_max) {
    return (value < in_min) ? in_min : ((value > in_max) ? in_max : value);
  }

  template <typename T> constexpr const T & Min(const T& in1, const T& in2, const T& in3) {
    return std::min(std::min(in1,in2), in3);
  }

  // Build a min and max that allows a variable number of inputs to be compared.
  template <typename T> const T & Min(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto min_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it < *min_found) min_found = it;
    }
    return *min_found;
  }

  template <typename T> const T & Max(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto max_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it > *max_found) max_found = it;
    }
    return *max_found;
  }

  template <typename T>
  static inline std::vector<T> BuildRange(T min, T max, T step=1) {
    size_t size = (size_t) ((max-min) / step);
    emp_assert(size >= 0);
    std::vector<T> out_v((size_t) size);
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

  static std::string UniqueName(const std::string & prefix="",
                                        const std::string & postfix="") {
    std::stringstream ss;
    ss << prefix << UniqueVal() << postfix;
    return ss.str();
  }
}

#endif
