//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::valsort_map

#include <set>
#include <string>

#include "emp/datastructs/valsort_map.hpp"
#include "emp/math/Random.hpp"

int main()
{
  constexpr size_t num_vals = 20;
  emp::valsort_map<size_t, double> test_map;
  emp::Random random;

  for (size_t i = 0; i < num_vals; i++) {
    test_map.Set(i, random.GetDouble(-100.0, 100.0));
  }

  std::cout << "INSERT ORDER:" << std::endl;
  for (auto it = test_map.cbegin(); it != test_map.cend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }

  std::cout << "\nSORTED ORDER:" << std::endl;
  for (auto it = test_map.cvbegin(); it != test_map.cvend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }

  emp::valsort_map<std::string, int> score_map;
  score_map.Set("Tytalus", 11);
  score_map.Set("Tremere", 10);
  score_map.Set("Bonisagus", 12);
  score_map.Set("Verditius", 7);
  score_map.Set("Mercere", 8);
  score_map.Set("ExMisc", 3);
  score_map.Set("Criamon", -608);
  score_map.Set("Diedne", 4);
  score_map.Set("Flambeau", 5);
  score_map.Set("Jerbiton", 8);
  score_map.Set("Merinita", 4);
  score_map.Set("Quaesitor", 0);



  std::cout << "\n\nBY NAME:\n";
  for (auto it = score_map.cbegin(); it != score_map.cend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }

  std::cout << "\nBY HIGH SCORE\n";
  for (auto it = score_map.crvbegin(); it != score_map.crvend(); it++) {
    std::cout << it->first << " : " << it->second << std::endl;
  }
}
