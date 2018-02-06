/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Trait.cc
 *  @brief An example file for Trait and TraitSet classes.
 */

#include "data/Trait.h"

int main()
{
  emp::Trait<double, double> test_trait("times_10", [](double & x){ return x*2; });
}
