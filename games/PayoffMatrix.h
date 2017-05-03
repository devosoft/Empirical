//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A simple game theory payoff matrix.


#ifndef EMP_GAME_PAYOFF_MATRIX_H
#define EMP_GAME_PAYOFF_MATRIX_H

#include "../base/array.h"

namespace emp {

  // A simple payoff matrix for a symmetric two-player game.
  class PayoffMatrix {
  private:
    emp::vector<double> payoff;
    size_t num_moves;

    constexpr size_t to_index(size_t A, size_t B) const { return A * num_moves + B; }
  public:
    PayoffMatrix(size_t _moves=2) : num_moves(_moves) {
      const size_t combos = num_moves * num_moves;
      payoff.resize(combos);
    }

    double & operator()(size_t A, size_t B) { return payoff[to_index(A,B)]; }
  };

}

#endif
