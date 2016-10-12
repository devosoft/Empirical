//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build NK-based algorithms.

#ifndef EMP_EVO_NK_H
#define EMP_EVO_NK_H

#include <array>

#include "../tools/BitVector.h"
#include "../tools/math.h"
#include "../tools/memo_function.h"
#include "../tools/Random.h"
#include "../tools/vector.h"

namespace emp {
namespace evo {

  class NKLandscape {
  private:
    const uint32_t N;
    const uint32_t K;
    const uint32_t state_count;
    const uint32_t total_count;
    emp::vector< emp::vector<double> > landscape;

  public:
    NKLandscape() = delete;
    NKLandscape(const NKLandscape &) = delete;
    NKLandscape(NKLandscape &&) = default;
    NKLandscape(int _N, int _K, emp::Random & random)
     : N(_N), K(_K)
     , state_count(emp::IntPow<uint32_t>(2,K+1))
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

    int GetN() const { return N; }
    int GetK() const { return K; }
    int GetStateCount() const { return state_count; }
    int GetTotalCount() const { return total_count; }

    double GetFitness(int n, uint32_t state) const {
      emp_assert(state < state_count, state, state_count);
      return landscape[n][state];
    }
    double GetFitness( std::vector<uint32_t> states ) const {
      emp_assert(states.size() == N);
      double total = landscape[0][states[0]];
      for (int i = 1; i < (int) N; i++) total += GetFitness(i,states[i]);
      return total;
    }
    double GetFitness(BitVector genome) const {
      emp_assert(genome.GetSize() == (int) N);

      // Use a double-length genome to easily handle wrap-around.
      genome.Resize(N*2);
      genome |= (genome << N);

      double total = 0.0;
      uint32_t mask = emp::MaskLow<uint32_t>(K+1);
      for (int i = 0; i < (int) N; i++) {
        const uint32_t cur_val = (genome >> i).GetUInt(0) & mask;
	      const double cur_fit = GetFitness(i, cur_val);
        total += cur_fit;
      }
      return total;
    }
  };

  class NKLandscapeMemo {
  private:
    const int N;
    const int K;
    mutable emp::vector< emp::memo_function<double(const BitVector &)> > landscape;
    mutable std::unordered_map<BitVector, double> fit_cache;
    emp::vector<BitVector> masks;

  public:
    NKLandscapeMemo() = delete;
    NKLandscapeMemo(const NKLandscapeMemo &) = delete;
    NKLandscapeMemo(NKLandscapeMemo &&) = default;
    NKLandscapeMemo(int _N, int _K, emp::Random & random) : N(_N), K(_K), landscape(N), masks(N)
    {
      // Each position in the landscape...
      for (int n = 0; n < N; n++) {
        // ...should have its own memo_function
        landscape[n] = [&random](const BitVector &){ return random.GetDouble(); };
        // ...and its own mask.
        masks[n].Resize(N);
        for (int k = 0; k < K; k++) masks[n][(n+k)%N] = 1;
      }
    }
    ~NKLandscapeMemo() { ; }
    NKLandscapeMemo & operator=(const NKLandscapeMemo &) = delete;
    NKLandscapeMemo & operator=(NKLandscapeMemo &&) = default;

    int GetN() const { return N; }
    int GetK() const { return K; }

    double GetFitness(int n, const BitVector & state) const {
      emp_assert(state == (state & masks[n]));
      return landscape[n](state);
    }
    double GetFitness(const BitVector & genome) const {
      emp_assert(genome.GetSize() == N);

      // If this fitness is cached, return it!
      const auto it = fit_cache.find(genome);
      if (it != fit_cache.end()) return it->second;

      // Otherwise calculate it.
      double total = 0.0;
      for (int n = 0; n < N; n++) {
        total += landscape[n](genome & masks[n]);
      }
      fit_cache[genome] = total;
      return total;
    }
  };

}
}

#endif
