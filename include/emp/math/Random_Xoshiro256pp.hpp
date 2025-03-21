/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief A high-quality and fast pseudo-random-number generator engine.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_XOSHIRO256PP_HPP_INCLUDE
#define EMP_MATH_RANDOM_XOSHIRO256PP_HPP_INCLUDE

#include "../base/array.hpp"
#include "../base/assert.hpp"

namespace emp {

  // The xoshiro256++ PRNG engine is based on XOR shifts, rotations, and simple arithmetic.
  // In: PRNGs by Blackman & Vigna
  struct Random_Xoshiro256pp {
    emp::array<uint64_t, 4> state;  // Internal state for RNG

    // HELPER FUNCTIONS

    // Rotates left by k bits
    static uint64_t rotl(const uint64_t value, int rot) noexcept {
      emp_assert(rot > 0 && rot < 64);
      return (value << rot) | (value >> (64 - rot));
    }

    [[nodiscard]] std::string GetType() const noexcept { return "Random_Xoshiro256pp"; }

    // Basic 64bit Random number
    uint64_t Get() noexcept {
      uint64_t result = rotl(state[0] + state[3], 23) + state[0];

      uint64_t t = state[1] << 17;

      state[2] ^= state[0];
      state[3] ^= state[1];
      state[1] ^= state[2];
      state[0] ^= state[3];

      state[2] ^= t;
      state[3] = rotl(state[3], 45);

      return result;
    }

    /// Starts a new sequence of pseudo random numbers; seed must be > 0.
    void ResetSeed(uint64_t seed) noexcept {
      emp_assert(seed > 0);

      // Simple splitmix64 seeding (adapted from Blackman & Vigna)      
      uint64_t tmp;
      for (size_t i = 0; i < 4; ++i) {
        tmp = (seed += 0x9e3779b97f4a7c15); // Shift seed for each starting point.
        tmp = (tmp ^ (tmp >> 30)) * 0xbf58476d1ce4e5b9;
        tmp = (tmp ^ (tmp >> 27)) * 0x94d049bb133111eb;
        state[i] = tmp ^ (tmp >> 31);
      }
    }

  };

} // END emp namespace

#endif // #ifndef EMP_MATH_RANDOM_XOSHIRO256PP_HPP_INCLUDE
