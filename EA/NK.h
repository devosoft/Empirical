//  This file is part of Empirical, https://github.com/mercere99/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This file provides code to build NK-based algorithms.

#ifndef NK_H
#define NK_H

#include <array>

#include "../tools/const_utils.h"
#include "../tools/Random.h"

namespace emp {
namespace EA {

  template <int N, int K>
  class NKLandscape {
  private:
    static constexpr int state_count = emp::constant::IntPow(2,K);
    std::array< std::array<double, state_count>, N > landscape;

  public:
    NKLandscape() { ; }
    NKLandscape(emp::Random & random) {
      for (double & pos : landscape) pos = random.GetDouble();
    }
    NKLandscape(const NKLandscape &) = default;
    ~NKLandscape() { ; }
    NKLandscape & operator=(const NKLandscape &) = default;

    constexpr int GetN() const { return N; }
    constexpr int GetK() const { return K; }
    constexpr int GetStateCount() const { return state_count; }

    double Get(int n, uint32_t state) { return landscape[n][state]; }
  };
  
}
}
  
#endif

