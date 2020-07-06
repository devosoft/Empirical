/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020
 *
 *  @file  hash_utils.h
 *  @brief This file provides tools for hashing values and containers.
 *  @note Status: BETA
 */

#ifndef EMP_HASH_UTILS_H
#define EMP_HASH_UTILS_H

#include <type_traits>
#include <cstddef>
#include <stdint.h>

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
    return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
  }

  constexpr size_t rotate(const size_t x, const size_t r)
  {
    return (x << r) | (x >> (64 - r));
  }

  constexpr inline void fmix64(size_t& k)
  {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;
  }

  constexpr size_t inline murmur_hash(
    const std::byte* key,
    const size_t numbytes,
    const size_t seed = 0
  ) {
    // define constants and starting seeds
    size_t h1 = seed;
    size_t h2 = seed;

    const size_t c1 = 0x87c37b91114253d5LLU;
    const size_t c2 = 0x4cf5ad432745937fLLU;

    // main algorithm loop
    for (size_t i = 0; i < numbytes / 2; i++) {
      size_t k1 = static_cast<size_t>(key[2 * i]);
      size_t k2 = static_cast<size_t>(key[2 * i + 1]);

      k1 *= c1;
      k1 = rotate(k1, 31);
      k1 *= c2;
      h1 ^= k1;

      h1 = rotate(h1, 27);
      h1 += h2;
      h1 = 5 * h1 + 0x52dce729;

      k2 *= c2;
      k2 = rotate(k2, 33);
      k2 *= c1;
      h2 ^= k2;

      h2 = rotate(h2, 31);
      h2 += h1;
      h2 = 5 * h2 + 0x38495ab5;
    }

    // finalization
    h1 ^= numbytes;
    h2 ^= numbytes;

    h1 += h2;
    h2 += h1;

    fmix64(h1);
    fmix64(h2);

    h1 += h2;

    return h1;
}
  template <typename Container, size_t Seed = 0>
  struct ContainerHash
  {
    size_t operator()(const Container& v) const {
        size_t data = Seed;
        for (const auto& x : v) {
            using T = typename std::decay<decltype(x)>::type;
            const std::hash<T> hasher;
            data = hash_combine(data, hasher(x));
        }
        return data;
    }
  };
}

#endif
