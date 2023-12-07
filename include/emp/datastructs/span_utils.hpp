/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file span_utils.hpp
 *  @brief A set of simple functions to manipulate std::span
 *  @note Status: BETA
 *
 */

#ifndef EMP_DATASTRUCTS_SPAN_UTILS_HPP_INCLUDE
#define EMP_DATASTRUCTS_SPAN_UTILS_HPP_INCLUDE

#include <iostream>
#include <span>

#include "../base/array.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// Print the contents of a span.
  template <typename T>
  void Print(const std::span<T> & v, std::ostream & os=std::cout, const std::string & spacer=" ") {
    for (size_t id = 0; id < v.size(); id++) {
      if (id) os << spacer; // Put a space before second element and beyond.
      os << emp::to_string(v[id]);
    }
  }

  /// Convert an emp::array to an equivalent span
  template <typename T, size_t SIZE>
  auto to_span(emp::array<T,SIZE> a) { return std::span<T,SIZE>(a); }

  /// Convert an emp::vector to an equivalent span
  template <typename T, typename... Ts>
  auto to_span(emp::vector<T,Ts...> v) { return std::span<T>(v); }
}

namespace std {
  // A generic streaming function for spans.
  template <typename T>
  std::ostream & operator<<(std::ostream & out, std::span<T> s) {
    emp::Print(s, out);
    return out;
  }

  template <typename T>
  std::istream & operator>>(std::istream & is, std::span<T> s) {
    for (T & x : s) is >> x;
    return is;
  }

}

#endif // #ifndef EMP_DATASTRUCTS_SPAN_UTILS_HPP_INCLUDE
