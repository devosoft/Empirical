#ifndef POLYFILL_SPAN_H
#define POLYFILL_SPAN_H

#include "../../../third-party/span-lite/include/nonstd/span.hpp"

// alias span-lite's nonstd::span to std::span
// this is done to ease transition to C++20 spans at a later point
// TODO: C++20 || cpp20
namespace std {
  template <typename ...Args>
  using span = nonstd::span<Args...>;
}

#endif
