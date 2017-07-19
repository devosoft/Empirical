//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build NK-based algorithms.

#ifndef EMP_EVO_NK_H
#define EMP_EVO_NK_H

#include <array>

#include "../base/vector.h"
#include "../tools/BitVector.h"
#include "../tools/math.h"
#include "../tools/memo_function.h"
#include "../tools/Random.h"

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
  /// that is larger than will fit in memory. If you are using small values for N and K,
  /// you can get better performance by using an NKLandscapeConst instead.

  class NKLandscape {
  private:
    const size_t N;
    const size_t K;
    const size_t state_count;
    const size_t total_count;
    emp::vector< emp::vector<double> > landscape;

  public:
    NKLandscape() = delete;
    NKLandscape(const NKLandscape &) = delete;
    NKLandscape(NKLandscape &&) = default;

    /// N is the length of bitstrings in your population, K is the number of neighboring sites
    /// the affect the fitness contribution of each site (i.e. epistasis or ruggedness), random
    /// is the random number generator to use to generate this landscape.
    NKLandscape(size_t _N, size_t _K, emp::Random & random)
     : N(_N), K(_K)
     , state_count(emp::IntPow<size_t>(2,K+1))
     , total_count(N * state_count)
     , landscape(N)
    {
      emp_assert(K < 32, K); // Genes will be stored in a 32-bit int; consider using NKLandscape Memo!
      for ( auto & ltable : landscape) {
        ltable.resize(state_count);
        for (double & pos : ltable) {
          pos = random.GetDouble();
        }
      }
    }
    ~NKLandscape() { ; }
    NKLandscape & operator=(const NKLandscape &) = delete;
    NKLandscape & operator=(NKLandscape &&) = default;

    /// Returns N
    size_t GetN() const { return N; }
    /// Returns K
    size_t GetK() const { return K; }
    /// Get the number of posssible states for a given site
    size_t GetStateCount() const { return state_count; }
    /// Get the total number of states possible in the landscape
    /// (i.e. the number of different fitness contributions in the table)
    size_t GetTotalCount() const { return total_count; }

    /// Get the fitness contribution of position [n] when it (and its K neighbors) have the value
    /// [state]
    double GetFitness(size_t n, size_t state) const {
      emp_assert(state < state_count, state, state_count);
      return landscape[n][state];
    }

    /// Get the fitness of a whole  bitstring
    double GetFitness( std::vector<size_t> states ) const {
      emp_assert(states.size() == N);
      double total = landscape[0][states[0]];
      for (size_t i = 1; i < N; i++) total += GetFitness(i,states[i]);
      return total;
    }

    /// Get the fitness of a whole bitstring (pass by value so can be modified.)
    double GetFitness(BitVector genome) const {
      emp_assert(genome.GetSize() == N, genome.GetSize(), N);

      // Use a double-length genome to easily handle wrap-around.
      genome.Resize(N*2);
      genome |= (genome << N);

      double total = 0.0;
      size_t mask = emp::MaskLow<size_t>(K+1);
      for (size_t i = 0; i < N; i++) {
        const size_t cur_val = (genome >> i).GetUInt(0) & mask;
	      const double cur_fit = GetFitness(i, cur_val);
        total += cur_fit;
      }
      return total;
    }
  };

  /// The NKLandscapeMemo class is simialar to NKLandscape, but it does not pre-calculate all
  /// of the landscape states.  Instead it determines the value of each gene combination on first
  /// use and memorizes it.

  class NKLandscapeMemo {
  private:
    const size_t N;
    const size_t K;
    mutable emp::vector< emp::memo_function<double(const BitVector &)> > landscape;
    emp::vector<BitVector> masks;

  public:
    NKLandscapeMemo() = delete;
    NKLandscapeMemo(const NKLandscapeMemo &) = delete;
    NKLandscapeMemo(NKLandscapeMemo &&) = default;
    NKLandscapeMemo(size_t _N, size_t _K, emp::Random & random)
      : N(_N), K(_K), landscape(N), masks(N)
    {
      // Each position in the landscape...
      for (size_t n = 0; n < N; n++) {
        // ...should have its own memo_function
        landscape[n] = [&random](const BitVector &){ return random.GetDouble(); };
        // ...and its own mask.
        masks[n].Resize(N);
        for (size_t k = 0; k < K; k++) masks[n][(n+k)%N] = 1;
      }
    }
    ~NKLandscapeMemo() { ; }
    NKLandscapeMemo & operator=(const NKLandscapeMemo &) = delete;
    NKLandscapeMemo & operator=(NKLandscapeMemo &&) = default;

    size_t GetN() const { return N; }
    size_t GetK() const { return K; }

    double GetFitness(size_t n, const BitVector & state) const {
      emp_assert(state == (state & masks[n]));
      return landscape[n](state);
    }
    double GetFitness(const BitVector & genome) const {
      emp_assert(genome.GetSize() == N);

      // Otherwise calculate it.
      double total = 0.0;
      for (size_t n = 0; n < N; n++) {
        total += landscape[n](genome & masks[n]);
      }
      return total;
    }
  };

}
}

#endif
