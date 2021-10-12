//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::ra_set

#include <set>

#include "emp/datastructs/ra_set.hpp"
#include "emp/math/Random.hpp"

int main()
{
  const int num_vals = 100000;

  emp::vector<int> vals;
  emp::vector<int> vals_remove;
  emp::Random random;

  for (int i = 0; i < num_vals; i++) {
    vals.push_back(random.GetInt(num_vals));
    vals_remove.push_back(random.GetInt(num_vals));
  }

  emp::ra_set<int> ra_vals;
  for (int x : vals) ra_vals.insert(x);
  for (int x : vals_remove) ra_vals.erase(x);

  std::set<int> set_vals;
  for (int x : vals) set_vals.insert(x);
  for (int x : vals_remove) set_vals.erase(x);

  std::cout << "ra size = " << ra_vals.size() << std::endl;
  std::cout << "set size = " << set_vals.size() << std::endl;

}
