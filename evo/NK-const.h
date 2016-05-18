//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build NK-based algorithms.

#ifndef EMP_EVO_NK_CONST_H
#define EMP_EVO_NK_CONST_H

#include <array>

#include "../tools/assert.h"
#include "../tools/BitSet.h"
#include "../tools/const_utils.h"
#include "../tools/Random.h"

namespace emp {
namespace evo {

  template <int N, int K>
  class NKLandscapeConst {
  private:
    static constexpr int state_count = emp::constant::IntPow(2,K+1);
    static constexpr int total_count = N * state_count;
    std::array< std::array<double, state_count>, N > landscape;

  public:
    NKLandscapeConst() = delete; // { ; }
    NKLandscapeConst(emp::Random & random) {
      for ( std::array<double, state_count> & ltable : landscape) {
        for (double & pos : ltable) {
          pos = random.GetDouble();
        }
      }
    }
    NKLandscapeConst(const NKLandscapeConst &) = delete;
    ~NKLandscapeConst() { ; }
    NKLandscapeConst & operator=(const NKLandscapeConst &) = delete;

    constexpr int GetN() const { return N; }
    constexpr int GetK() const { return K; }
    constexpr int GetStateCount() const { return state_count; }
    constexpr int GetTotalCount() const { return total_count; }

    double GetFitness(int n, uint32_t state) const {
      emp_assert(state < state_count, state, state_count);
      // std::cout << n << " : " << state << " : " << landscape[n][state] << std::endl;
      return landscape[n][state];
    }
    double GetFitness( std::array<uint32_t, N> states ) const {
      double total = landscape[0][states[0]];
      for (int i = 1; i < N; i++) total += GetFitness(i,states[i]);
      return total;
    }
    double GetFitness(const BitSet<N> & genome) const {
      // Create a double-length genome to easily handle wrap-around.
      BitSet<N*2> genome2( genome.template Export<N*2>() );
      genome2 |= (genome2 << N);

      double total = 0.0;
      constexpr uint32_t mask = emp::constant::MaskLow<uint32_t>(K+1);
      for (int i = 0; i < N; i++) {
	      const uint32_t cur_val = (genome2 >> i).GetUInt(0) & mask;
	      const double cur_fit = GetFitness(i, cur_val);
	      total += cur_fit;
      }
      return total;
    }
  };

}
}

#endif
