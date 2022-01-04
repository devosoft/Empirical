/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file PayoffMatrix.hpp
 *  @brief A simple game theory payoff matrix.
 *
 */

#ifndef EMP_GAMES_PAYOFFMATRIX_HPP_INCLUDE
#define EMP_GAMES_PAYOFFMATRIX_HPP_INCLUDE

#include <unordered_map>

#include "../base/array.hpp"
#include "../base/vector.hpp"

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

    using move_t = size_t;

    void Reset() { ; } // No history...

    void SetVal(size_t self, size_t other, double value) { payoff[to_index(self,other)] = value; }
    double & operator()(size_t A, size_t B) { return payoff[to_index(A,B)]; }
    double operator()(size_t A, size_t B) const { return payoff[to_index(A,B)]; }

    // Setup a Prisoner's Dilema Payoff Matrix
    void SetupPD(double u) {
      payoff[to_index(0,0)] = u;         // Both defect
      payoff[to_index(0,1)] = 1.0 + u;   // Player defects, other cooperates
      payoff[to_index(1,0)] = 0.0;       // Player cooperates, other defects
      payoff[to_index(1,1)] = 1.0;       // Both cooperate
    }

    void SetupSnowdrift(double cost) {
      payoff[to_index(0,0)] = 0;            // Both defect -- trapped in snowdrift
      payoff[to_index(0,1)] = 1.0;          // Player defects, other digs out alone.
      payoff[to_index(1,0)] = 1.0 - cost;   // Player digs out by themself.
      payoff[to_index(1,1)] = 1.0 - cost/2; // Both dig out together and share cost.
    }

    std::unordered_map<int, double> AsInput(size_t player_id) const {
      std::unordered_map<int, double> out_map;
      out_map[-1] = num_moves;
      for (size_t i = 0; i < payoff.size(); i++) {
        out_map[i] = payoff[i];
      }
      return out_map;
    }

  };

}

#endif // #ifndef EMP_GAMES_PAYOFFMATRIX_HPP_INCLUDE
