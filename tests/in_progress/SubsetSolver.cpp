/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file SubsetSolver.cpp
 */

#include <chrono>
#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/SubsetSolver.hpp"

TEST_CASE("Test SubsetSolver", "[tools]")
{
  emp::SubsetSolver<true> ss(10);
  ss.SetEvalFun([](const emp::SolveState & state) -> double { return state.CountIn(); });

  CHECK(ss.Solve() == 10.0);

  emp::SubsetSolver<true> ss2(22);
  ss2.SetEvalFun([](const emp::SolveState & state) -> double {
    auto test_v = state.GetInVector();
    size_t count = (~(test_v << 2) & ~(test_v << 1) & test_v & ~(test_v >> 1) & ~(test_v >> 2)).CountOnes();
    return static_cast<double>(count);
  });

  ss2.SetIncludeOptFun([](emp::SolveState & state, size_t id){
    if (id > 0) state.Exclude(id-1);
    if (id > 1) state.Exclude(id-2);
    if (id < state.GetSize()-1) state.Exclude(id+1);
    if (id < state.GetSize()-2) state.Exclude(id+2);
  });

  auto start = std::chrono::high_resolution_clock::now();
  CHECK(ss2.Solve() == 8.0);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  std::cout << "Duration = " << duration/1000.0 << " seconds." << std::endl;
}
