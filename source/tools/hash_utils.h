/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  hash_utils.h
 *  @brief This file provides tools for hashing values.
 *  @note Status: BETA
 */

#ifndef EMP_HASH_UTILS_H
#define EMP_HASH_UTILS_H

#include <type_traits>

namespace emp {

  /// mixin: the additional value to mix in to the hash soup
  /// acc: the accumulated value
  /// RE implementation see, e.g.,
  /// https://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
  size_t combine_hash(size_t mixin, size_t acc) {
    return mixin + 0x9e3779b9 + (acc << 19) + (acc >> 13);
  }

  /// generate a unique long from a pair of ints
  uint64_t szudzik_hash(uint32_t a_, uint32_t b_)
  {
    uint64_t a = a_, b = b_;
    return a >= b ? a * a + a + b : a + b * b;
  }

}

#endif
