/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2025
*/
/**
 *  @file
 *  @brief Helper functions for emp::Random for common random tasks.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_UTILS_HPP_INCLUDE
#define EMP_MATH_RANDOM_UTILS_HPP_INCLUDE

#include <functional>
#include <stddef.h>

#include "../base/vector.hpp"
#include "../bits/BitVector.hpp"

namespace emp {

  /// Choose a random element from an indexable container.
  template <typename T, typename RAND_T>
  inline auto SelectRandom(RAND_T & random, const T & container) {
    return container[random.GetUInt(container.size())];
  }

  /// Randomly reorder all of the elements in a vector.
  /// If max_count is provided, just make sure that the first max_count entries are randomly
  /// drawn from entire vector.

  template <typename T, typename RAND_T>
  inline void Shuffle(RAND_T & random, emp::vector<T> & v, size_t max_count)
  {
    emp_assert(max_count <= v.size());
    for (size_t i = 0; i < max_count; i++) {
      const size_t pos = random.GetUInt(i, v.size());
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
    }
  }

  template <typename T, typename RAND_T>
  inline void Shuffle(RAND_T & random, emp::vector<T> & v) { Shuffle(random, v, v.size()); }

  template <typename T, typename RAND_T>
  inline void ShuffleRange(RAND_T & random, emp::vector<T> & v, size_t first, size_t last)
  {
    emp_assert(first <= last);
    emp_assert(last <= v.size());
    for (size_t i = first; i < last; i++) {
      const size_t pos = random.GetUInt(i, last);
      if (pos == i) continue;
      std::swap(v[i], v[pos]);
    }
  }

  /// Return an emp::vector<int> numbered 0 through size-1 in a random order.

  inline emp::vector<size_t> GetPermutation(auto & random, size_t size) {
    emp::vector<size_t> seq(size);
    seq[0] = 0;
    for (size_t i = 1; i < size; i++) {
      size_t val_pos = random.GetUInt(i+1);
      seq[i] = seq[val_pos];
      seq[val_pos] = i;
    }
    return seq;
  }

  /// Choose K positions from N possibilities.

  inline void Choose(auto & random, size_t N, size_t K, std::vector<size_t> & choices) {
    emp_assert (N >= K);

    choices.resize(K);
    while (K) {
      if (N==K || random.P(((double) K)/((double) N))) { choices[--K] = --N; }
      else --N;
    }
  }

  inline std::vector<size_t> Choose(auto & random, size_t N, size_t K) {
    std::vector<size_t> choices;
    Choose(random,N,K,choices);
    return choices;
  }


  /// Generate a random BitVector of the specified size.
  inline BitVector RandomBitVector(auto & random, size_t size, double p=0.5)
  {
    emp_assert(p >= 0.0 && p <= 1.0);
    BitVector bits(size);
    for (size_t i = 0; i < size; i++) bits[i] = random.P(p);
    return bits;
  }

  /// Generate a random double vector in the specified range.
  inline emp::vector<double> RandomDoubleVector(auto & random, size_t size, double min, double max) {
    emp::vector<double> vals(size);
    for (double & v : vals) v = random.GetDouble(min, max);
    return vals;
  }

  /// Generate a random size_t vector in the specified range.
  inline emp::vector<size_t> RandomUIntVector(auto & random, size_t size, size_t min, size_t max) {
    emp::vector<size_t> vals(size);
    for (size_t & v : vals) v = random.GetUInt(min, max);
    return vals;
  }

  /// Generate a random vector in the specified type and range.
  template <typename T>
  inline emp::vector<T> RandomVector(auto & random, size_t size, T min, T max) {
    emp::vector<T> vals(size);
    for (T & v : vals) v = (T) random.GetDouble((double) min, (double) max);
    return vals;
  }

  /// Generate a random BitVector of the specified size.
  inline void RandomizeBitVector(BitVector & bits, auto & random, double p=0.5)
  {
    emp_assert(p >= 0.0 && p <= 1.0);
    for (size_t i = 0; i < bits.size(); i++) bits[i] = random.P(p);
  }

  /// Generate a random vector in the specified type and range.
  template <typename T, typename RAND_T>
  inline void RandomizeVector(emp::vector<T> & vals, RAND_T & random, T min, T max) {
    for (T & v : vals) v = (T) random.GetDouble((double) min, (double) max);
  }

  template <typename RAND_T>
  inline size_t CountRngTouches(std::function< void( RAND_T & ) > routine) {
    RAND_T rand_baseline{ 1 };
    RAND_T rand_comparison{ 1 };

    routine( rand_baseline );
    const auto after_routine = rand_baseline.GetUInt();

    size_t count{};
    while( rand_comparison.GetUInt() != after_routine ) ++count;

    return count;
  }

  // Use random::P() in all cases.
  class RandProbability {
  private:
    const double p;
  public:
    RandProbability(double p) : p(p) { }
    template <typename RAND_T>
    bool Test(RAND_T & random) {
      return random.P(p);
    }
  };
  
  class LowProbability {
  private:
    const double scale;
    uint32_t next=0;

    double CalcScale(double p) const {
      // emp_assert(p < 0.25, "Ideally, low probabilities are below 0.05");
      return 1.0 / std::log(1.0 - p);
    }
  public:
    LowProbability(double p) : scale(CalcScale(p)) {
      emp_assert(p > 0.0 && p < 1.0);
    }
    template <typename RAND_T>
    bool Test(RAND_T & random) {
      if (!next) {
        const double U = random.GetDouble();
        next = (U==0.0) ? 1 : (static_cast<size_t>(std::log(U) * scale) + 1);
      }
      return !(--next);
    }
  };

  class HighProbability {
  private:
    const double scale;
    size_t next=0;

    double CalcScale(double p) const {
      // emp_assert(p > 0.75, "Ideally, high probabilities are above 0.95");
      return 1.0 / std::log(p);
    }
  public:
    HighProbability(double p) : scale(CalcScale(p)) {
      emp_assert(p > 0.0 && p < 1.0);     
    }
    template <typename RAND_T>
    bool Test(RAND_T & random) {
      if (!next) {
        const double U = random.GetDouble();
        next = (U==0.0) ? 1 : (static_cast<size_t>(std::log(U) * scale) + 1);
      }
      return --next;
    }
  };
  
  class BufferedProbability {
  private:
    static constexpr uint32_t BUFFER_SIZE=256;
    const double p;
    emp::array<uint8_t, BUFFER_SIZE> buffer;
    uint32_t next = BUFFER_SIZE;

    template <typename RAND_T>
    void ResetBuffer(RAND_T & random) {
      for (uint8_t & buffer_entry : buffer) {
        buffer_entry = random.P(p);
      }
      next = 0;
    }
  public:
    BufferedProbability(double p) : p(p) {
      emp_assert(p > 0.0 && p < 1.0);
    }
    template <typename RAND_T>
    bool Test(RAND_T & random) {
      if (next == BUFFER_SIZE) ResetBuffer(random);
      ++next;
      return buffer[next-1];
    }
  };

  // DEVELOPER NOTE:
  // - Pre-recording probabilities with bits was always slower.
  // - Distribution objects didn't help above calculating distribution on the fly.



// TIMINGS WITH JUST NEGATIVE BINOMIAL DISTRIBUTION TRICK.
// >>>> p=0.0005:
//   RAND: Hits = 999912; Frac = 0.000499956; Time = 2.17961 sec
//   PROB: Hits = 3994305; Frac = 0.00199715; Time = 0.651069 sec
// >>>> p=0.001:
//   RAND: Hits = 1998946; Frac = 0.000999473; Time = 2.1727 sec
//   PROB: Hits = 5403631; Frac = 0.00270182; Time = 0.674695 sec
// >>>> p=0.0025:
//   RAND: Hits = 5002913; Frac = 0.00250146; Time = 2.19858 sec
//   PROB: Hits = 9435543; Frac = 0.00471777; Time = 0.747944 sec
// >>>> p=0.005:
//   RAND: Hits = 10004482; Frac = 0.00500224; Time = 2.2464 sec
//   PROB: Hits = 15698119; Frac = 0.00784906; Time = 0.840072 sec
// >>>> p=0.01:
//   RAND: Hits = 20005738; Frac = 0.0100029; Time = 2.33296 sec
//   PROB: Hits = 27278354; Frac = 0.0136392; Time = 0.98746 sec
// >>>> p=0.025:
//   RAND: Hits = 50000160; Frac = 0.0250001; Time = 2.58263 sec
//   PROB: Hits = 59991759; Frac = 0.0299959; Time = 1.51072 sec
// >>>> p=0.05:
//   RAND: Hits = 99991050; Frac = 0.0499955; Time = 3.00382 sec
//   PROB: Hits = 112109317; Frac = 0.0560547; Time = 2.36058 sec
// >>>> p=0.1:
//   RAND: Hits = 200005554; Frac = 0.100003; Time = 3.85344 sec
//   PROB: Hits = 214660238; Frac = 0.10733; Time = 4.02593 sec
// >>>> p=0.25:
//   RAND: Hits = 500030635; Frac = 0.250015; Time = 6.45553 sec
//   PROB: Hits = 516742068; Frac = 0.258371; Time = 8.85098 sec
// >>>> p=0.5:
//   RAND: Hits = 999988451; Frac = 0.499994; Time = 10.3042 sec
//   PROB: Hits = 1019490639; Frac = 0.509745; Time = 13.3259 sec
}

#endif // #ifndef EMP_MATH_RANDOM_UTILS_HPP_INCLUDE
