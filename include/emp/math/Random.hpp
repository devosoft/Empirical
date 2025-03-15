/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2015-2025.
*/
/**
 *  @file
 *  @brief A versatile and non-patterned pseudo-random-number generator.
 *  Status: RELEASE
 */

#ifndef EMP_MATH_RANDOM_HPP_INCLUDE
#define EMP_MATH_RANDOM_HPP_INCLUDE

#include "Random_Xoshiro256pp.hpp"
#include "Random_MSWS.hpp"

namespace emp {

  using Random = emp::Random_Xoshiro256pp;

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
