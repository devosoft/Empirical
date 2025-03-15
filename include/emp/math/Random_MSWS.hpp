/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025 (forked from on Random.hpp)
*/
/**
 *  @file
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_MSWS_HPP_INCLUDE
#define EMP_MATH_RANDOM_MSWS_HPP_INCLUDE

#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iterator>
#include <stddef.h>

#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../bits/bitset_utils.hpp"

#include "Range.hpp"

namespace emp {

  ///  Middle Square Weyl Sequence: A versatile and non-patterned pseudo-random-number
  ///  generator. https://arxiv.org/abs/1704.00358
  ///  Based on: https://en.wikipedia.org/wiki/Middle-square_method
  class Random_MSWS {
  protected:

    uint64_t value = 0;                       ///< Current squaring value
    uint64_t weyl_state = 0;                  ///< Weyl sequence state
    uint64_t value2 = 0;                      ///< Extra squaring value for 64 bit
    uint64_t weyl_state2 = 0;                 ///< Extra Weyl sequence state for 64 bit
    uint64_t original_seed = 0;               ///< Seed to start sequence; initialized weyl_state

    // Members & functions for stat functions
    double expRV = 0.0;    ///< Exponential Random Variable for the randNormal function

    // Constants ////////////////////////////////////////////////////////////////
    static constexpr uint64_t VAL32_CAP   = uint64_t{1} << 32; // 4'294'967'296 (1^32)
    static constexpr double   VAL32_CAP_D = static_cast<double>(VAL32_CAP);
    static constexpr double   VAL32_FRAC  = 1.0 / VAL32_CAP_D;
    static constexpr uint64_t VAL53_CAP   = uint64_t{1} << 53; // 9'007'199'254'740'992 (1^53)
    static constexpr double   VAL53_CAP_D = static_cast<double>(VAL53_CAP);
    static constexpr double   VAL53_FRAC  = 1.0 / VAL53_CAP_D;

    static constexpr uint64_t STEP_SIZE   = 0xb5ad4eceda1ce2a9; // Weyl sequence step size
    static constexpr uint64_t STEP_SIZE2  = 0x278c5a4d8419fe6b; // Extra step size for 64 bit

    static constexpr unsigned char BYTE1 = (unsigned char) 1;

    /// Basic Random number
    /// Returns a random number [0, VAL32_CAP)
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

    /// Returns a random number [0, max-64-bit-uint)
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
    
  public:
    /// Set up the random generator object with an optional seed value.
    Random_MSWS(const int seed = -1) noexcept {
      ResetSeed(seed);  // Calls init()
    }

    ~Random_MSWS() { ; }

    /// Advance pseudorandom number generation engine one step.
    void StepEngine() noexcept { Get(); }

    /// @return The current seed used to initialize this pseudo-random sequence.
    [[nodiscard]] uint64_t GetSeed() const noexcept { return original_seed; }

    /// Starts a new sequence of pseudo random numbers.  A negative seed means that the random
    /// number generator gets its seed from the current system time and the process memory.
    void ResetSeed(const int64_t seed) noexcept {
      value = 0;
      value2 = 0;
      expRV = 0.0;

      // If the provided seed is <= 0, choose a unique seed based on time and memory location.
      if (seed <= 0) {
        uint64_t seed_time = static_cast<uint64_t>(time(NULL));
        uint64_t seed_mem = std::bit_cast<uint64_t>(this);
        weyl_state2 = weyl_state = seed_time ^ seed_mem;
      }

      else weyl_state2 = weyl_state = static_cast<uint64_t>(seed);

      // Save the seed that was ultimately used to start this pseudo-random sequence.
      original_seed = weyl_state;

      weyl_state *= 2;  // Make sure starting state is even.
      weyl_state2 *= 2;

      Get(); // Prime the new sequence by skipping the first number.
    }


    // Random Number Generation /////////////////////////////////////////////////

    /// @return A pseudo-random double value between [0.0, 1.0)
    [[nodiscard]] double GetDouble() noexcept { return Get() * VAL32_FRAC; }

    /// @return A pseudo-random double value between [0.0, max)
    [[nodiscard]] double GetDouble(const double max) noexcept { return GetDouble() * max; }

    /// @return A pseudo-random double value between [min, max)
    [[nodiscard]] double GetDouble(const double min, const double max) noexcept {
      return GetDouble() * (max - min) + min;
    }

    /// @return A pseudo-random double in the provided range.
    [[nodiscard]] double GetDouble(const Range<double> range) noexcept {
      return GetDouble(range.GetLower(), range.GetUpper());
    }

    /// @return A pseudo-random double value between (0.0, 1.0)
    [[nodiscard]] double GetDoubleNonZero() noexcept {
      double result;
      do { result = GetDouble(); } while (result == 0.0);
      return result;
    }

    /// @return A higher precision (53 bit) pseudo-random double value between [0.0, 1.0)
    [[nodiscard]] double GetDouble64() noexcept { return Get() * VAL32_FRAC; }

    /// @return A pseudo-random double value between [0.0, max)
    [[nodiscard]] double GetDouble64(const double max) noexcept { return GetDouble64() * max; }

    /// @return A pseudo-random double value between [min, max)
    [[nodiscard]] double GetDouble64(const double min, const double max) noexcept {
      return GetDouble64() * (max - min) + min;
    }

    /// @return A pseudo-random double in the provided range.
    [[nodiscard]] double GetDouble64(const Range<double> range) noexcept {
      return GetDouble64(range.GetLower(), range.GetUpper());
    }

    /// @return A pseudo-random double value between (0.0, 1.0)
    [[nodiscard]] double GetDouble64NonZero() noexcept {
      double result;
      do { result = GetDouble64(); } while (result == 0.0);
      return result;
    }
    

    /// @return A pseudo-random 32-bit (4 byte) unsigned int value.
    [[nodiscard]] uint32_t GetUInt() noexcept { return Get(); }

    /// @return A pseudo-random 32-bit unsigned int value between [0, max)
    template <typename T>
    [[nodiscard]] uint32_t GetUInt(const T max) noexcept {
      return static_cast<uint32_t>(GetDouble() * static_cast<double>(max));
    }

    /// @return A pseudo-random 32-bit unsigned int value between [min, max)
    template <typename T1, typename T2>
    [[nodiscard]] uint32_t GetUInt(const T1 min, const T2 max) noexcept {
      return GetUInt<uint32_t>((uint32_t) max - (uint32_t) min) + (uint32_t) min;
    }

    /// @return A pseudo-random 32-bit unsigned int value in the provided range.
    template <typename T>
    [[nodiscard]] uint32_t GetUInt(const Range<T> range) noexcept {
      return GetUInt(range.GetLower(), range.GetUpper());
    }


    /// @return A pseudo-random 32 bits (unsigned int) with a 12.5% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits12_5() noexcept { return Get() & Get() & Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 25% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits25() noexcept { return Get() & Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 37.5% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits37_5() noexcept { return (Get() | Get()) & Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 50% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits50() noexcept { return Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 62.5% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits62_5() noexcept { return (Get() & Get()) | Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 75% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits75() noexcept { return Get() | Get(); }

    /// @return A pseudo-random 32 bits (unsigned int) with a 87.5% chance of each bit being 1.
    [[nodiscard]] uint32_t GetBits87_5() noexcept { return Get() | Get() | Get(); }


    /// @return A pseudo-random 64-bit (8 byte) unsigned int value.
    [[nodiscard]] uint64_t GetUInt64() noexcept {
      return Get64();
    }

    /// @return A pseudo-random 64-bit unsigned int value between 0 and max
    [[nodiscard]] uint64_t GetUInt64(const uint64_t max) noexcept {
      if (max <= VAL32_CAP) return GetUInt(max);      // Don't need extra precision.

      uint64_t mask = emp::MaskUsed(max);            // Create a mask for just the bits we need.
      uint64_t val = GetUInt64() & mask;             // Grab a value using just the current bits.
      while (val >= max) val = GetUInt64() & mask;   // Grab new values until we find a valid one.

      return val;
    }

    /// @return A pseudo-random 64-bit unsigned int value between 0 and max
    [[nodiscard]] uint64_t GetUInt64(const uint64_t min, const uint64_t max) noexcept {
      return GetUInt64(max - min) + min;
    }
    

    /// @return A pseudo-random 32-bit (4 byte) int value between 0 and max
    [[nodiscard]] int32_t GetInt(const int32_t max) noexcept {
      return static_cast<int32_t>(GetUInt((uint32_t) max));
    }

    /// @return A pseudo-random 32-bit (4 byte) int value between min and max
    [[nodiscard]] int32_t GetInt(const int min, const int max) noexcept {
      return GetInt(max - min) + min;
    }

    /// @return A pseudo-random 32-bit (4 byte) int value in range
    [[nodiscard]] int32_t GetInt(const Range<int> range) noexcept {
      return GetInt(range.GetLower(), range.GetUpper());
    }

    /// Enumeration for common probabilities.
    /// (not class, so can be referred to elsewhere as e.g., Random::PROB_50)
    enum Prob { PROB_0   = 0,   PROB_12_5 = 125,
                PROB_25  = 250, PROB_37_5 = 375,
                PROB_50  = 500, PROB_62_5 = 625,
                PROB_75  = 750, PROB_87_5 = 875,
                PROB_100 = 1000 };

    /// Shortcut type for all functions that deal witch chunks of memory.
    using mem_ptr_t = emp::Ptr<unsigned char>;

    /// Randomize a contiguous segment of memory.
    void RandFill(mem_ptr_t dest, const size_t num_bytes) noexcept {
      dest.FillMemoryFunction( num_bytes, [this](){ return Get(); } );
    }

    /// Randomize a contiguous segment of memory.
    template <Prob PROB>
    void RandFillP(mem_ptr_t dest, const size_t num_bytes) noexcept {
      if constexpr (PROB == PROB_0) {
        dest.FillMemoryFunction( num_bytes, [](){ return 0; } );
      } else if constexpr (PROB == PROB_12_5) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits12_5(); } );
      } else if constexpr (PROB == PROB_25) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits25(); } );
      } else if constexpr (PROB == PROB_37_5) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits37_5(); } );
      } else if constexpr (PROB == PROB_50) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits50(); } );
      } else if constexpr (PROB == PROB_62_5) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits62_5(); } );
      } else if constexpr (PROB == PROB_75) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits75(); } );
      } else if constexpr (PROB == PROB_87_5) {
        dest.FillMemoryFunction( num_bytes, [this](){ return GetBits87_5(); } );
      } else if constexpr (PROB == PROB_100) {
        dest.FillMemoryFunction( num_bytes, [](){ return static_cast<size_t>(-1); } );
      }
    }

    /// Randomize a contiguous segment of memory between specified bit positions.
    template <Prob PROB>
    void RandFillP(mem_ptr_t dest, [[maybe_unused]] const size_t num_bytes, size_t start_bit, size_t stop_bit) noexcept
    {
      emp_assert(start_bit <= stop_bit);
      emp_assert(stop_bit <= num_bytes*8);

      const size_t start_byte_id = start_bit >> 3;     // At which byte do we start?
      const size_t end_byte_id = stop_bit >> 3;        // At which byte do we stop?
      const size_t start_bit_id = start_bit & 7;       // Which bit to start at in byte?
      const size_t end_bit_id = stop_bit & 7;          // Which bit to stop before in byte?
      constexpr double p = static_cast<double>(PROB) / 1000.0;   // Determine actual probability of a 1

      // If the start byte and end byte are the same, just fill those in.
      if (start_byte_id == end_byte_id) {
        for (size_t i = start_bit_id; i < end_bit_id; ++i) {
          uint8_t mask = (uint8_t) (1 << i);
          if (P(p)) dest[start_byte_id] |= mask;
          else dest[start_byte_id] &= ~mask;
        }
        return;
      }

      const uint8_t start_byte = dest[start_byte_id];    // Save first byte to restore bits.

      // Randomize the full bits we need to use.
      RandFillP<PROB>(dest + start_byte_id, end_byte_id - start_byte_id);

      // If we are not starting at the beginning of a byte, restore missing bits.
      if (start_bit_id) {
        const uint8_t mask = (uint8_t) ((1 << start_bit_id) - 1); // Signify how byte is divided.
        (dest[start_byte_id] &= ~mask) |= (start_byte & mask);    // Stitch together byte parts.
      }

      // If we have a byte at the end to partially randomize, do so.
      if (end_bit_id) {
        uint8_t & end_byte = dest[end_byte_id];                 // Grab reference to end byte
        const uint8_t mask = (uint8_t) ((1 << end_bit_id) - 1); // Signify how byte is divided.
        end_byte &= ~mask;                                      // Clear out bits to be randomized.
        for (size_t i = 0; i < end_bit_id; i++) {               // Step through bits to flip.
          if (P(p)) end_byte |= ((uint8_t) 1 << i);             // Set appropriate bits.
        }
      }
    }

    // Shortcuts to randomize a contiguous segment of memory with fixed probabilities of a 1.
    void RandFill0(   mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_0>   (dest, bytes); }
    void RandFill12_5(mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_12_5>(dest, bytes); }
    void RandFill25(  mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_25>  (dest, bytes); }
    void RandFill37_5(mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_37_5>(dest, bytes); }
    void RandFill50(  mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_50>  (dest, bytes); }
    void RandFill62_5(mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_62_5>(dest, bytes); }
    void RandFill75(  mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_75>  (dest, bytes); }
    void RandFill87_5(mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_87_5>(dest, bytes); }
    void RandFill100( mem_ptr_t dest, const size_t bytes) noexcept { RandFillP<PROB_100> (dest, bytes); }

    void RandFill0(   mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_0>   (dest, bytes, start_bit, stop_bit); }
    void RandFill12_5(mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_12_5>(dest, bytes, start_bit, stop_bit); }
    void RandFill25(  mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_25>  (dest, bytes, start_bit, stop_bit); }
    void RandFill37_5(mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_37_5>(dest, bytes, start_bit, stop_bit); }
    void RandFill50(  mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_50>  (dest, bytes, start_bit, stop_bit); }
    void RandFill62_5(mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_62_5>(dest, bytes, start_bit, stop_bit); }
    void RandFill75(  mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_75>  (dest, bytes, start_bit, stop_bit); }
    void RandFill87_5(mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_87_5>(dest, bytes, start_bit, stop_bit); }
    void RandFill100( mem_ptr_t dest, const size_t bytes, size_t start_bit, size_t stop_bit) noexcept
      { RandFillP<PROB_100> (dest, bytes, start_bit, stop_bit); }

    /// Randomize a contiguous segment of memory with a given probability of each bit being on.
    void RandFill(mem_ptr_t dest, const size_t num_bytes, const double p) noexcept {
      // Try to find a shortcut if p allows....
      if (p == 0.0)        return RandFill0(dest, num_bytes);
      else if (p == 0.125) return RandFill12_5(dest, num_bytes);
      else if (p == 0.25)  return RandFill25(dest, num_bytes);
      else if (p == 0.375) return RandFill37_5(dest, num_bytes);
      else if (p == 0.5)   return RandFill50(dest, num_bytes);
      else if (p == 0.625) return RandFill62_5(dest, num_bytes);
      else if (p == 0.75)  return RandFill75(dest, num_bytes);
      else if (p == 0.875) return RandFill87_5(dest, num_bytes);
      else if (p == 1.0)   return RandFill100(dest, num_bytes);

      // This is not a special value of P, so let's set each bit manually
      // (slow, but good for now; ideally use closest faster option above and modify)
      for (size_t i = 0; i < num_bytes; i++) dest[i] = GetByte(p);
    }

    /// Randomize a contiguous segment of memory with a given probability of each bit being on.
    void RandFill(mem_ptr_t dest, const size_t num_bytes, const double p,
                  const size_t start_bit, const size_t stop_bit) noexcept {
      emp_assert((stop_bit >> 3) <= num_bytes);

      // Try to find a shortcut if p allows....
      if (p == 0.0)        return RandFill0(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.125) return RandFill12_5(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.25)  return RandFill25(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.375) return RandFill37_5(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.5)   return RandFill50(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.625) return RandFill62_5(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.75)  return RandFill75(dest, num_bytes, start_bit, stop_bit);
      else if (p == 0.875) return RandFill87_5(dest, num_bytes, start_bit, stop_bit);
      else if (p == 1.0)   return RandFill100(dest, num_bytes, start_bit, stop_bit);

      // This is not a special value of P, so let's set each bit manually
      // (slow, but good for now; ideally use closest faster option above and modify)
      size_t cur_byte = start_bit >> 3;
      uint8_t cur_mask = (uint8_t) (1 << (start_bit & 7));
      for (size_t i = start_bit; i < stop_bit; i++) {
        if (P(p)) dest[cur_byte] |= cur_mask;     // Set the target bit.
        else dest[cur_byte] &= ~cur_mask;         // Clear out the target bit.
        cur_mask <<= 1;                           // Move to the next bit.
        if (!cur_mask) {                          // If the next bit is out of this byte...
          cur_byte++;                             //   move to the next byte.
          cur_mask = 1;                           //   reset the mask.
        }
      }
    }


    // Random Event Generation //////////////////////////////////////////////////

    /// Tests a random value [0,1) against a given probability p, and returns true of false.
    /// @param p The probability of the result being "true".
    [[nodiscard]] bool P(const double p) noexcept {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      return (Get() < p * VAL32_CAP_D);
    }

    /// Full random byte with each bit being a one with a given probability.
    [[nodiscard]] unsigned char GetByte(const double p) noexcept {
      unsigned char out_byte = 0;
      if (P(p)) out_byte |= 1;
      if (P(p)) out_byte |= 2;
      if (P(p)) out_byte |= 4;
      if (P(p)) out_byte |= 8;
      if (P(p)) out_byte |= 16;
      if (P(p)) out_byte |= 32;
      if (P(p)) out_byte |= 64;
      if (P(p)) out_byte |= 128;
      return out_byte;
    }


    // Statistical functions ////////////////////////////////////////////////////

    // Distributions //

    /// Generate a random variable drawn from a unit normal distribution.
    [[nodiscard]] double GetNormal() noexcept {
      // Draw from a Unit Normal Dist
      // Using Rejection Method and saving of initial exponential random variable
      double expRV2;
      while (1) {
        expRV2 = -log(GetDoubleNonZero());
        expRV -= (expRV2-1)*(expRV2-1)/2;
        if (expRV > 0) break;
        expRV = -log(GetDoubleNonZero());
      }
      if (P(.5)) return expRV2;
      return -expRV2;
    }

    /// @return A random variable drawn from a normal distribution.
    /// @param mean Center of distribution.
    /// @param std Standard deviation of distribution.
    [[nodiscard]] double GetNormal(const double mean, const double std) { return mean + GetNormal() * std; }

    /// Generate a random variable drawn from a Poisson distribution.
    [[nodiscard]] uint32_t GetPoisson(const double n, const double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      // Optimizes for speed and calculability using symetry of the distribution
      if (p > .5) return (uint32_t) n - GetPoisson(n * (1 - p));
      else return GetPoisson(n * p);
    }

    /// Generate a random variable drawn from a Poisson distribution.
    [[nodiscard]] uint32_t GetPoisson(const double mean) {
      // Draw from a Poisson Dist with mean; if cannot calculate, return UINT_MAX.
      // Uses Rejection Method
      const double a = exp(-mean);
      if (a <= 0) return UINT_MAX; // cannot calculate, so return UINT_MAX
      uint32_t k = 0;
      double u = GetDouble();
      while (u >= a) {
        u *= GetDouble();
        ++k;
      }
      return k;
    }

    /// Generate a random variable drawn from a Binomial distribution.
    ///
    /// This function is exact, but slow.
    /// @see Random_MSWS::GetApproxRandBinomial
    /// @see emp::Binomial in source/tools/Distribution.h
    [[nodiscard]] uint32_t GetBinomial(const double n, const double p) { // Exact
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Actually try n Bernoulli events, each with probability p
      uint32_t k = 0;
      for (uint32_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
    }

    /// Generate a random variable drawn from an exponential distribution.
    [[nodiscard]] double GetExponential(double p) {
      emp_assert(p > 0.0 && p <= 1.0, p);
      // if (p == 0.0) return std::numeric_limits<double>::infinity();
      if (p == 1.0) return 0.0;
      return std::log(GetDouble()) / std::log(1.0 - p);
    }

    /// Generate a random variable drawn from a geometric distribution.
    [[nodiscard]] uint32_t GetGeometric(double p) {
      emp_assert(p > 0.0 && p <= 1.0, p);
      return static_cast<uint32_t>( GetExponential(p) ) + 1;
    }

  };


  // /// This is an adaptor to make Random_MSWS behave like a proper STL random number generator.
  // struct RandomStdAdaptor {
  //   typedef int argument_type;
  //   typedef int result_type;

  //   RandomStdAdaptor(Random_MSWS & rng) : _rng(rng) { }
  //   int operator()(int n) { return _rng.GetInt(n); }

  //   Random_MSWS & _rng;
  // };


  // /// Draw a sample (with replacement) from an input range, copying to the output range.
  // template <typename ForwardIterator, typename OutputIterator, typename RNG>
  // void sample_with_replacement(ForwardIterator first,
  //                              ForwardIterator last,
  //                              OutputIterator ofirst,
  //                              OutputIterator olast,
  //                              RNG & rng) noexcept {
  //   std::size_t range = std::distance(first, last);
  //   while(ofirst != olast) {
  //     *ofirst = *(first+rng(range));
  //     ++ofirst;
  //   }
  // }


} // END emp namespace

#endif // #ifndef EMP_MATH_RANDOM_MSWS_HPP_INCLUDE
