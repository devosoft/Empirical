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

    void SetVal(size_t self, size_t other, double value) { payoff[to_index(self,other)] = value; }
    double & operator()(size_t A, size_t B) { return payoff[to_index(A,B)]; }
    double operator()(size_t A, size_t B) const { return payoff[to_index(A,B)]; }

    SetupPD(double u) {
      payoff[to_index(0,0)] = u;         // Both defect
      payoff[to_index(0,1)] = 1.0 + u;   // Player defects, other cooperates
      payoff[to_index(1,0)] = 0.0;       // Player cooperates, other defects
      payoff[to_index(1,1)] = 1.0;       // Both cooperate
    }
  };

}

#endif
