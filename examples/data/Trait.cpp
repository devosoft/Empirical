/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Trait.cc
 *  @brief An example file for Trait and TraitSet classes.
 */

#include "emp/data/Trait.hpp"

int main()
{
  emp::Trait<double, double> test_trait("times_10", [](double & x){ return x*10; });

  emp::TraitSet<double> test_set;
  test_set.AddTrait<double>("times_2", [](double & x){ return x*2; });
  test_set.AddTrait<double>("times_3", [](double & x){ return x*3; });
  test_set.AddTrait<double>("times_4", [](double & x){ return x*4; });

  double in = 100.0;
  auto results = test_set.EvalValues(in);
  for (auto x : results) std::cout << x << std::endl;
}
