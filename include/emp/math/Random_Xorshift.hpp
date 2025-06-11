/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/math/Random_Xorshift.hpp
 * @brief A FAST pseudo-random-number generator that has a short (2^64) period.
 * Status: RELEASE
 */

#pragma once

#ifndef INCLUDE_EMP_MATH_RANDOM_XORSHIFT_HPP_GUARD
#define INCLUDE_EMP_MATH_RANDOM_XORSHIFT_HPP_GUARD

 #include "../base/assert.hpp"

 namespace emp {

  // The Xorshift random number generators step through all 64-bit values for a
  // 2^64 period.  The weaker randomness comes with much faster computation.
  // Not for encryption, but good for simulations.
  struct Random_Xorshift {
    uint64_t state;  // Internal state for RNG

    [[nodiscard]] std::string GetType() const noexcept { return "Random_Xorshift"; }

    // Basic 64bit Random number
    uint64_t Get() noexcept {
      state ^= state >> 12;
      state ^= state << 25;
      state ^= state >> 27;
      return state * 2685821657736338717ULL;
    }

    /// Starts a new sequence of pseudo random numbers; seed must be > 0.
    void ResetSeed(uint64_t seed) noexcept {
      emp_assert(seed > 0);
      state = seed ^ 0x9e3779b97f4a7c15;
    }

  };

} // END emp namespace

 #endif // #ifndef EMP_MATH_RANDOM_XORSHIFT_HPP_INCLUDE
