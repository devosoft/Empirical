/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file SubsetSolver.hpp
 *  @brief A brute-force solving tool to find an optimal subset.
 *  @note Status: BETA
 */

#ifndef EMP_TOOLS_SUBSET_SOLVER_HPP_INCLUDE
#define EMP_TOOLS_SUBSET_SOLVER_HPP_INCLUDE

#include <functional>
#include <stddef.h>

#include "../base/assert.hpp"

#include "SolveState.hpp"

namespace emp {

  template <bool FIND_MAX=false>
  class SubsetSolver {
  private:
    double best_score;
    SolveState best_state;

    /// Function to identify the value of a provided solution.
    using eval_fun_t = std::function<double(const SolveState &)>;
    eval_fun_t eval_fun;

    /// Function to identify the next entry to consider for inclusion / exclusion
    using next_id_fun_t = std::function<size_t(const SolveState &)>;
    next_id_fun_t next_id_fun;

    /// Function to do polynomial-time optimization after an include.
    using include_opt_fun_t = std::function<void(SolveState &, size_t id)>;
    include_opt_fun_t include_opt_fun;

    void UpdateBest(const SolveState & state, double score) {
      best_score = score;
      best_state = state;
    }

    void EvalSolution(const SolveState & state) {
      const double result = eval_fun(state);
      if constexpr (FIND_MAX) {
        if (result > best_score) { UpdateBest(state, result); }
      } else {
        if (result < best_score) { UpdateBest(state, result); }
      }
    }

  public:
    SubsetSolver(size_t problem_size) : best_state(problem_size) {
      next_id_fun = [](const SolveState & state){ return static_cast<size_t>(state.GetNextUnk()); };
    }

    void SetEvalFun(eval_fun_t in_fun) { eval_fun = in_fun; }
    void SetNextIDFun(next_id_fun_t in_fun) { next_id_fun = in_fun; }
    void SetIncludeOptFun(include_opt_fun_t in_fun) { include_opt_fun = in_fun; }

    void Solve(SolveState state) {
      if (state.IsFinal()) {
        EvalSolution(state);
        return;
      }

      size_t id = next_id_fun(state);
      state.Exclude(id);
      Solve(state);
      state.Include(id);
      if (include_opt_fun) include_opt_fun(state, id);
      Solve(state);
    }

    double Solve() {
      best_state.Reset();
      Solve(best_state);
      return best_score;
    }
  };

}

#endif // #ifndef EMP_TOOLS_SUBSET_SOLVER_HPP_INCLUDE
