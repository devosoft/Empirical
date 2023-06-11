/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file span.hpp
 *  @brief Polyfill for C++20 std::span.
 */

#ifndef EMP_POLYFILL_SPAN_HPP_INCLUDE
#define EMP_POLYFILL_SPAN_HPP_INCLUDE

#if __cplusplus <= 201703L

#include "../../../third-party/span-lite/include/nonstd/span.hpp"
// alias span-lite's nonstd::span to std::span
// this is done to ease transition to C++20 spans at a later point
// TODO: C++20 || cpp20
namespace std {
  template <typename ...Args>
  using span = nonstd::span<Args...>;
}

#else // #if __cplusplus <= 201703L

#include <span>

#endif // #if __cplusplus <= 201703L

#endif // #ifndef EMP_POLYFILL_SPAN_HPP_INCLUDE
