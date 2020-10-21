/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  NK-const.hpp
 *  @brief This file provides code to build NK landscapes, setup at compile time..
 *
 *  Knowing the size of N and K at compile time allow for slightly more optimized code, at the
 *  expense of flexibility.
 */

#ifndef EMP_EVO_NK_CONST_H
#define EMP_EVO_NK_CONST_H

#include <array>

#include "../base/assert.hpp"
#include "../bits/BitSet.hpp"
#include "../math/math.hpp"
#include "../math/Random.hpp"

namespace emp {
namespace evo {


  /// An NK Landscape is a popular tool for studying theoretical questions about evolutionary
  /// dynamics. It is a randomly generated fitness landscape on which bitstrings can evolve.
  /// NK Landscapes have two parameters: N (the length of the bitstrings) and K (epistasis).
  /// Since you have control over the amount of epistasis, NK Landscapes are often called
  /// "tunably rugged" - a useful feature, since the ruggedness of the fitness landscape is thought
  /// to be important to many evolutionary dynamics. For each possible value that a site and its
  /// K neighbors to the right can have, a random fitness contribution is chosen.
  /// These contributions are summed across the bitstring. So when K = 0, each site has a single
  /// optimal value, resulting in a single smooth fitness peak.
  ///
  /// For more information, see Kauffman and Levin,
  /// 1987 (Towards a general theory of adaptive walks on rugged landscapes).
  ///
  /// This object handles generating and maintaining an NK fitness landscape.
  /// Note: Overly large Ns and Ks currently trigger a seg-fault, caused by trying to build a table
  /// that is larger than will fit in memory. You can use larger values of N and K (for slightly
  /// reduced speed) of you use an NKLandscape object instead.

  template <size_t N, size_t K>
  class NKLandscapeConst {
  private:
    static constexpr size_t state_count() { return emp::IntPow<size_t>(2,K+1); }
    static constexpr size_t total_count() { return N * state_count(); }
    std::array< std::array<double, state_count()>, N > landscape;

  public:
    NKLandscapeConst() = delete; // { ; }

    /// Build a new NKLandscapeConst using the random number generator [random]
    NKLandscapeConst(emp::Random & random) : landscape() {
      for ( std::array<double, state_count()> & ltable : landscape) {
        for (double & pos : ltable) {
          pos = random.GetDouble();
        }
      }
    }
    NKLandscapeConst(const NKLandscapeConst &) = delete;
    ~NKLandscapeConst() { ; }
    NKLandscapeConst & operator=(const NKLandscapeConst &) = delete;

    /// Returns N
    constexpr size_t GetN() const { return N; }
    /// Returns K
    constexpr size_t GetK() const { return K; }
    /// Get the number of posssible states for a given site
    constexpr size_t GetStateCount() const { return state_count(); }
    /// Get the total number of states possible in the landscape
    /// (i.e. the number of different fitness contributions in the table)
    constexpr size_t GetTotalCount() const { return total_count(); }

    /// Get the fitness contribution of position [n] when it (and its K neighbors) have the value
    /// [state]
    double GetFitness(size_t n, size_t state) const {
      emp_assert(state < state_count(), state, state_count());
      // std::cout << n << " : " << state << " : " << landscape[n][state] << std::endl;
      return landscape[n][state];
    }

    /// Get the fitness of a whole  bitstring
    double GetFitness( std::array<size_t, N> states ) const {
      double total = landscape[0][states[0]];
      for (size_t i = 1; i < N; i++) total += GetFitness(i,states[i]);
      return total;
    }

    /// Get the fitness of a whole  bitstring
    double GetFitness(const BitSet<N> & genome) const {
      // Create a double-length genome to easily handle wrap-around.
      BitSet<N*2> genome2( genome.template Export<N*2>() );
      genome2 |= (genome2 << N);

      double total = 0.0;
      constexpr size_t mask = emp::MaskLow<size_t>(K+1);
      for (size_t i = 0; i < N; i++) {
	      const size_t cur_val = (genome2 >> (int)i).GetUInt(0) & mask;
	      const double cur_fit = GetFitness(i, cur_val);
	      total += cur_fit;
      }
      return total;
    }
  };

}
}

#endif
