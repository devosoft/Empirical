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

  /// generate a unique long from a pair of ints
  uint64_t szudzik_hash(uint32_t a_, uint32_t b_)
  {
    uint64_t a = a_, b = b_;
    return a >= b ? a * a + a + b : a + b * b;
  }
  //https://www.boost.org/doc/libs/1_37_0/doc/html/hash/reference.html#boost.hash_combine
  constexpr inline std::size_t hash_combine(std::size_t hash1, std::size_t hash2)
  {
    return hash1 ^ (hash2 * 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
  }

}

#endif
