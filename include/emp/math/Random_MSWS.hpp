/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025
*/
/**
 *  @file
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_MSWS_HPP_INCLUDE
#define EMP_MATH_RANDOM_MSWS_HPP_INCLUDE

#include "../base/assert.hpp"

namespace emp {

  ///  Middle Square Weyl Sequence: A solid 32-bit pseudo-random-number engine
  ///  generator. https://arxiv.org/abs/1704.00358
  ///  Based on: https://en.wikipedia.org/wiki/Middle-square_method
  struct Random_MSWS {
    uint64_t value = 0;           ///< Current squaring value
    uint64_t weyl_state = 0;      ///< Weyl sequence state
    uint64_t value2 = 0;          ///< Extra squaring value for 64 bit
    uint64_t weyl_state2 = 0;     ///< Extra Weyl sequence state for 64 bit

    static constexpr uint64_t STEP_SIZE   = 0xb5ad4eceda1ce2a9; // Weyl sequence step size
    static constexpr uint64_t STEP_SIZE2  = 0x278c5a4d8419fe6b; // Extra step size for 64 bit

    [[nodiscard]] std::string GetType() const noexcept { return "Random_MSWS"; }
    
    /// Return a 32-bit Random number
    uint32_t Get() noexcept {
      value *= value;                       // Square the current value.
      value += (weyl_state += STEP_SIZE);   // Take a step in the Weyl sequence
      value = (value>>32) | (value<<32);    // Return the middle of the value
      return static_cast<uint32_t>(value);
    }

    // Get() from Paper:
    //
    //     uint64_t x = 0, w = 0, s = 0xb5ad4eceda1ce2a9;
    //     inline static uint32_t msws32() {
    //       x *= x; x += (w += s);
    //       return x = (x>>32) | (x<<32);
    //     }
    //
    // x => value
    // w => weyl_state
    // s => STEP_SIZE

    /// Return a 64-bit Random number
    uint64_t Get64() noexcept {
      // Square both values
      value *= value;  value2 *= value2;

      // Advance the weyl states and the values
      value += (weyl_state += STEP_SIZE);
      value2 += (weyl_state2 += STEP_SIZE2);

      // Backup the first value before swap (only swapped second value in result)
      const uint64_t nonswap_value = value;
      value = (value >> 32) | (value << 32);
      value2 = (value2 >> 32) | (value2 << 32);

      return nonswap_value ^ value2;
    }

    // Get64() from Paper:
    //
    //    uint64_t x1 = 0, w1 = 0, s1 = 0xb5ad4eceda1ce2a9;
    //    uint64_t x2 = 0, w2 = 0, s2 = 0x278c5a4d8419fe6b;
    //    inline static uint64_t msws64() {
    //      uint64_t xx;
    //      x1 *= x1; xx = x1 += (w1 += s1); x1 = (x1 >> 32) | (x1 << 32);
    //      x2 *= x2; x2 += (w2 += s2); x2 = (x2 >> 32) | (x2 << 32);
    //      return xx ^ x2;
    //    }
    

    /// Starts a new sequence of pseudo random numbers.  Seed is guaranteed > 0.
    void ResetSeed(const uint64_t seed) noexcept {
      value = value2 = 0;
      weyl_state = weyl_state2 = seed;

      weyl_state *= 2;  // Force starting state to be even.
      weyl_state2 *= 2;

      Get(); // Prime the new sequence by skipping the first number.
    }

  };

} // END emp namespace

#endif // #ifndef EMP_MATH_RANDOM_MSWS_HPP_INCLUDE
