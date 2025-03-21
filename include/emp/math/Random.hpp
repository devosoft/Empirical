/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2025.
*/
/**
 *  @file
 *  @brief A versatile pseudo-random-number generator.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_HPP_INCLUDE
#define EMP_MATH_RANDOM_HPP_INCLUDE

#include "Random_Xoshiro256pp.hpp"
#include "Random_MSWS.hpp"
#include "Random_Xorshift.hpp"

#include <cmath>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iterator>
#include <stddef.h>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../bits/bitset_utils.hpp"

#include "constants.hpp"
#include "Range.hpp"

namespace emp {

  // Random_Base provides extra functionality for a given RNG Engine (passed in
  // as a template parameter).
  //
  // The Engine must have:
  // - A constructor that accepts a random number seed (64 bit unsigned int, guaranteed non-zero).
  // - A `ResetSeed(uint64_t seed)` function that resets the engine appropriately.
  // - A `Get()` function that returns an unsigned int in its native type (32 or 64 bits)
  //
  // Optionally, if it has these other functions they will be used rather than creating
  // them internally:
  // - A `Get32()` returning a random 32-bit int (alternative: use last 32 bits when called)
  // - A `Get64()` returning a random 64-bit int (alternative: combine two 32-bit values)
  // - A `GetType()` returning an std::string with the type name for this engine.

  template <typename ENGINE_T>
  class Random_Base {
  private:
    ENGINE_T engine;
    static constexpr bool NATIVE64 = std::same_as<decltype(engine.Get()), uint64_t>;

    uint64_t original_seed = 0;     // Seed to start sequence

    // Members & functions for stat functions
    double expRV = 0.0;    ///< Exponential Random Variable for the randNormal function

    // Constants
    static constexpr uint64_t VAL32_CAP   = uint64_t{1} << 32; // 4'294'967'296 (1^32)
    static constexpr double   VAL32_CAP_D = static_cast<double>(VAL32_CAP);
    static constexpr double   VAL32_FRAC  = 1.0 / VAL32_CAP_D;

    static constexpr uint64_t VAL53_CAP   = uint64_t{1} << 53; // 9'007'199'254'740'992 (1^53)
    static constexpr double   VAL53_CAP_D = static_cast<double>(VAL53_CAP);
    static constexpr double   VAL53_FRAC  = 1.0 / VAL53_CAP_D;

    // Core functions
    uint32_t Get32() noexcept {
      if constexpr (requires { engine.Get32(); }) {
        return engine.Get32();
      } else {
        return static_cast<uint32_t>(Get());
      }
    }

    uint64_t Get64() noexcept {
      // Always prefer a call with the specified number of bits.
      if constexpr (requires { engine.Get64(); }) { return engine.Get64(); }
      
      // If the regular Get() is already 64 bits, use it.
      else if constexpr (NATIVE64) { return Get(); }

      // Otherwise stitch two 32-bit random values together.
      else { return (static_cast<uint64_t>(Get32()) << 32) + Get32(); }
    }

    // Get a value at the native resolution produced by the engine.
    auto Get() noexcept { return engine.Get(); }
   
  public:
    /// Set up the random generator object with an optional seed value (0 = based on time and ptr).
    Random_Base(uint64_t seed = 0) noexcept { ResetSeed(seed); }

    /// Advance pseudorandom number generation engine one step.
    void StepEngine() noexcept { Get(); }

    /// @return The current seed used to initialize this pseudo-random sequence.
    [[nodiscard]] uint64_t GetSeed() const noexcept { return original_seed; }

    /// @return The type name of the engine.
    [[nodiscard]] std::string GetEngineType() {
      if constexpr (requires { engine.GetType(); }) { return engine.GetType(); }
      return "Unknown";
    }

    /// Starts a new sequence of pseudo random numbers.  A negative seed means that the random
    /// number generator gets its seed from the current system time and the process memory.
    void ResetSeed(uint64_t seed) noexcept {
      expRV = 0.0;

      // If the provided seed is 0, choose a unique seed based on time and memory location.
      if (seed == 0) {
        uint64_t seed_time = static_cast<uint64_t>(time(NULL));
        uint64_t seed_mem = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
        seed_mem = (seed_mem << 32) + (seed_mem >> 32);
        seed = seed_time ^ seed_mem;
      }

      original_seed = seed;
      engine.ResetSeed(seed);
    }

    // Random Number Generation /////////////////////////////////////////////////

    /// @return A pseudo-random double value between [0.0, 1.0)
    [[nodiscard]] double GetDouble() noexcept {
      if constexpr (NATIVE64) {
        return (Get64() >> 11) * VAL53_FRAC;  // Doubles have 53 bits of precision.
      } else {
        return Get32() * VAL32_FRAC;
      }
    }

    /// @return A pseudo-random double value between [0.0, max)
    [[nodiscard]] double GetDouble(const double max) noexcept {
      return GetDouble() * max;
    }

    /// @return A pseudo-random double value between [min, max)
    [[nodiscard]] double GetDouble(const double min, const double max) noexcept {
      emp_assert(min < max, min, max);
      return GetDouble(max - min) + min;
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

    /// @return A pseudo-random 64-bit (8 byte) unsigned int value.
    [[nodiscard]] uint64_t GetUInt() noexcept { return Get64(); }

    /// @return A pseudo-random 64-bit unsigned int value between [0, max)
    template <typename T>
    [[nodiscard]] uint64_t GetUInt(const T max) noexcept {
      return static_cast<uint64_t>(GetDouble() * static_cast<double>(max));
    }

    /// @return A pseudo-random 64-bit unsigned int value between [min, max)
    template <typename T1, typename T2>
    [[nodiscard]] uint64_t GetUInt(const T1 min, const T2 max) noexcept {
      return GetUInt(max - min) + min;
    }

    /// @return A pseudo-random 64-bit unsigned int value in the provided range.
    template <typename T>
    [[nodiscard]] uint64_t GetUInt(const Range<T> range) noexcept {
      return GetUInt(range.GetLower(), range.GetUpper());
    }

    // == You can specify the number of bits in a UInt ==

    /// @return A pseudo-random 32-bit (4 byte) unsigned int value.
    [[nodiscard]] uint32_t GetUInt32() noexcept { return Get32(); }

    /// @return A pseudo-random 32-bit unsigned int value between [0, max)
    template <typename T>
    [[nodiscard]] uint32_t GetUInt32(const T max) noexcept {
      return static_cast<uint32_t>(GetDouble() * static_cast<double>(max));
    }

    /// @return A pseudo-random 32-bit unsigned int value between [min, max)
    template <typename T1, typename T2>
    [[nodiscard]] uint32_t GetUInt32(const T1 min, const T2 max) noexcept {
      return GetUInt<uint32_t>((uint32_t) max - (uint32_t) min) + (uint32_t) min;
    }

    /// @return A pseudo-random 32-bit unsigned int value in the provided range.
    template <typename T>
    [[nodiscard]] uint32_t GetUInt32(const Range<T> range) noexcept {
      return GetUInt(range.GetLower(), range.GetUpper());
    }


    /// @return A pseudo-random 64-bit (8 byte) unsigned int value.
    [[nodiscard]] uint64_t GetUInt64() noexcept { return Get64(); }

    /// @return A pseudo-random 64-bit unsigned int value between [0, max)
    /// This is a high-precision accessor with perfectly even probabilities in the range.
    inline uint64_t GetUInt64(const uint64_t max) noexcept {
      if (max <= VAL32_CAP) return GetUInt(max);    // Don't need extra precision.

      const uint64_t mask = emp::MaskUsed(max);     // Create a mask for just the bits we need.
      uint64_t val = GetUInt64() & mask;            // Grab a value using just the current bits.
      while (val >= max) val = GetUInt64() & mask;  // Grab new values until we find a valid one.

      return val;
    }

    /// @return A pseudo-random 64-bit unsigned int value between [min, max)
    template <typename T1, typename T2>
    [[nodiscard]] uint64_t GetUInt64(const T1 min, const T2 max) noexcept {
      return GetUInt64(max - min) + min;
    }

    /// @return A pseudo-random 64-bit unsigned int value in the provided range.
    template <typename T>
    [[nodiscard]] uint64_t GetUInt64(const Range<T> range) noexcept {
      return GetUInt(range.GetLower(), range.GetUpper());
    }

    /// @return A pseudo-random int value in [0, max)
    [[nodiscard]] int GetInt(const int max) noexcept {
      emp_assert(max > 0, "If you want a random negative int, specify both min and max", max);
      return static_cast<int>(GetUInt(static_cast<uint64_t>(max)));
    }

    /// @return A pseudo-random int value in [min, max)
    [[nodiscard]] int GetInt(const int min, const int max) noexcept {
      emp_assert(min < max, min, max);
      return GetInt(max - min) + min;
    }

    /// @return A pseudo-random int value in range
    [[nodiscard]] int GetInt(const Range<int> range) noexcept {
      return GetInt(range.GetLower(), range.GetUpper());
    }


    /// @return A pseudo-random 64 bits (unsigned int) with a 12.5% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits12_5() noexcept { return Get64() & Get64() & Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 25% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits25() noexcept { return Get64() & Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 37.5% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits37_5() noexcept { return (Get64() | Get64()) & Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 50% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits50() noexcept { return Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 62.5% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits62_5() noexcept { return (Get64() & Get64()) | Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 75% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits75() noexcept { return Get64() | Get64(); }

    /// @return A pseudo-random 64 bits (unsigned int) with a 87.5% chance of each bit being 1.
    [[nodiscard]] uint64_t GetBits87_5() noexcept { return Get64() | Get64() | Get64(); }


    /// Enumeration for common probabilities.
    /// (not class, so can be referred to as e.g., Random::PROB_50)
    enum Prob { PROB_0   = 0,   PROB_12_5 = 125,
                PROB_25  = 250, PROB_37_5 = 375,
                PROB_50  = 500, PROB_62_5 = 625,
                PROB_75  = 750, PROB_87_5 = 875,
                PROB_100 = 1000 };

    /// Shortcut type for all functions that deal with chunks of memory.
    using mem_ptr_t = emp::Ptr<uint8_t>;

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

    /// Randomize a contiguous segment of memory with a given probability of ones.
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

    /// Randomize a contiguous segment of memory with a given probability of ones.
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
      return (GetDouble() < p);
    }

    /// Full random byte with each bit being a one with a given probability.
    [[nodiscard]] uint8_t GetByte(const double p) noexcept {
      uint8_t out_byte = 0;
      for (unsigned i = 0; i < 8; ++i) {
        if (P(p)) out_byte |= (1 << i);
      }
      return out_byte;
    }

    // =======================  Statistical Functions  =======================

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
    [[nodiscard]] uint64_t GetPoisson(const double n, const double p) {
      emp_assert(p >= 0.0 && p <= 1.0, p);
      // Optimizes for speed and calculability using symmetry of the distribution
      if (p > .5) return static_cast<uint64_t>(n) - GetPoisson(n * (1 - p));
      else return GetPoisson(n * p);
    }

    /// Generate a random variable drawn from a Poisson distribution.
    [[nodiscard]] uint64_t GetPoisson(const double mean) {
      // Draw from a Poisson Dist with mean; if cannot calculate, return max value.
      // Uses Rejection Method
      const double a = exp(-mean);
      if (a <= 0) return MAX_8BYTE; // cannot calculate, so return max
      uint64_t k = 0;
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
    /// @see Random_Base::GetApproxRandBinomial
    /// @see emp::Binomial in source/tools/Distribution.h
    [[nodiscard]] uint64_t GetBinomial(const double n, const double p) { // Exact
      emp_assert(p >= 0.0 && p <= 1.0, p);
      emp_assert(n >= 0.0, n);
      // Actually try n Bernoulli events, each with probability p
      uint64_t k = 0;
      for (uint64_t i = 0; i < n; ++i) if (P(p)) k++;
      return k;
    }

    /// Generate a random variable drawn from an exponential distribution.
    [[nodiscard]] double GetExponential(double p) {
      emp_assert(p > 0.0 && p <= 1.0, p);
      if (p == 1.0) return 0.0;
      return std::log(GetDouble()) / std::log(1.0 - p);
    }

    /// Generate a random variable drawn from a geometric distribution.
    [[nodiscard]] uint64_t GetGeometric(double p) {
      emp_assert(p > 0.0 && p <= 1.0, p);
      return static_cast<uint64_t>( GetExponential(p) ) + 1;
    }

  };

  using RandomBest = Random_Base<emp::Random_Xoshiro256pp>;
  using Random32 = Random_Base<emp::Random_MSWS>;
  using RandomFast = Random_Base<emp::Random_Xorshift>;

  using Random = RandomBest;

  /// This is an adaptor to make Random behave like a Standard Library random number generator.
  struct RandomStdAdaptor {
    using argument_type = int;
    using result_type = int;

    RandomStdAdaptor(Random & rng) : _rng(rng) { }
    int operator()(int n) { return _rng.GetInt(n); }

    Random & _rng;
  };


  /// Draw a sample (with replacement) from an input range, copying to the output range.
  template <typename ForwardIterator, typename OutputIterator, typename RNG>
  void sample_with_replacement(ForwardIterator first,
                               ForwardIterator last,
                               OutputIterator ofirst,
                               OutputIterator olast,
                               RNG & rng) noexcept {
    std::size_t range = std::distance(first, last);
    while(ofirst != olast) {
      *ofirst = *(first+rng(range));
      ++ofirst;
    }
  }

} // END emp namespace

#endif // #ifndef EMP_MATH_RANDOM_HPP_INCLUDE
