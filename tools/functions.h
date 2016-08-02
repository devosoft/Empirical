//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  A collection of broadly-useful functions.

#ifndef EMP_FUNCTIONS_H
#define EMP_FUNCTIONS_H

#include <ctime>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <sstream>
#include <type_traits>
#include <vector>

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

  static double time_fun(std::function<void()> test_fun) {
    std::clock_t start_time = std::clock();
    test_fun();
    std::clock_t tot_time = std::clock() - start_time;
    return 1000.0 * ((double) tot_time) / (double) CLOCKS_PER_SEC;
  }

  // Toggle an input bool.
  inline bool toggle(bool & in_bool) { return (in_bool = !in_bool); }

  /// % is actually remainder; this is a proper modulus command that handles negative #'s correctly
  inline constexpr int mod(int in_val, int mod_val) {
    return (in_val < 0) ? (in_val % mod_val + mod_val) : (in_val % mod_val);
  }

  // A fast (O(log p)) integer-power command.
  static constexpr int pow(int base, int p) {
    return (p <= 0) ? 1 : base * pow(base, p-1);
  }


  // Run both min and max on a value to put it into a desired range.
  template <typename TYPE> constexpr TYPE to_range(const TYPE & value, const TYPE & in_min, const TYPE & in_max) {
    return (value < in_min) ? in_min : ((value > in_max) ? in_max : value);
  }

  template <typename T> constexpr const T & min(const T& in1, const T& in2, const T& in3) {
    return std::min(std::min(in1,in2), in3);
  }

  // Build a min and max that allows a variable number of inputs to be compared.
  template <typename T> const T & min(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto min_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it < *min_found) min_found = it;
    }
    return *min_found;
  }

  template <typename T> const T & max(std::initializer_list<const T&> lst) {
    emp_assert(lst.size > 0); // Nothing to return if nothing in the list!
    auto max_found = lst.begin();
    for (auto it = lst.begin()+1; it < lst.end(); it++) {
      if (*it > *max_found) max_found = it;
    }
    return *max_found;
  }


  static std::vector<int> build_range(int min, int max, int step=1) {
    int size = (max-min) / step;
    std::vector<int> out_v(size);
    int pos = 0;
    for (int i = min; i < max; i += step) {
      out_v[pos++] = i;
    }
    return out_v;
  }


  // The following two functions are from:
  // http://stackoverflow.com/questions/5056645/sorting-stdmap-using-value
  template<typename A, typename B> constexpr std::pair<B,A> flip_pair(const std::pair<A,B> &p)
  {
    return std::pair<B,A>(p.second, p.first);
  }

  template<typename A, typename B> std::multimap<B,A> flip_map(const std::map<A,B> &src)
  {
    std::multimap<B,A> dst;
    for (const auto & x : src) dst.insert( flip_pair(x) );
    return dst;
  }


  // Build a function to determine the size of a built-in array.
  template <typename T, size_t N>
  constexpr size_t size(T (&)[N]) { return N; }

}

#endif
