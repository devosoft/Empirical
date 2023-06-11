/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file IndexMap.cpp
 *  @brief Comparing the ordered versus unordered versions of IndexMap
 */

#include <ctime>         // For std::clock

#include "emp/base/vector.hpp"
#include "emp/datastructs/IndexMap.hpp"
#include "emp/datastructs/UnorderedIndexMap.hpp"
#include "emp/math/Random.hpp"

constexpr size_t NUM_WEIGHTS = 1000000;
constexpr size_t NUM_CALLS = 10000000;
constexpr double MAX_WEIGHT = 100.0;

int main()
{
  // Prepare some data.
  emp::Random random;
  std::vector<double> weights(NUM_WEIGHTS);
  std::vector<double> calls(NUM_CALLS);

  double total_weight = 0.0;
  for (double & x : weights) {
    x = std::pow(random.GetDouble(MAX_WEIGHT), 2.0);
    total_weight += x;
  }
  for (double & x : calls) {
    x = random.GetDouble(total_weight);
  }

  std::clock_t start_time1 = std::clock();

  // Setup this index_map.
  emp::IndexMap index_map(NUM_WEIGHTS);
  for (size_t id = 0; id < NUM_WEIGHTS; id++) index_map[id] = weights[id];

  // Run the tests.
  size_t total_ids = 0;
  for (double x : calls) total_ids += index_map.Index(x);

  std::clock_t tot_time1 = std::clock() - start_time1;
  std::cout << "Ordered total = " << total_ids
            << ";  time = " << 1000.0 * ((double) tot_time1) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;




  std::clock_t start_time2 = std::clock();

  // Setup this index_map.
  emp::UnorderedIndexMap index_map2(NUM_WEIGHTS);
  for (size_t id = 0; id < NUM_WEIGHTS; id++) index_map2[id] = weights[id];

  // Run the tests.
  size_t total_ids2 = 0;
  for (double x : calls) total_ids2 += index_map2.Index(x);

  std::clock_t tot_time2 = std::clock() - start_time2;
  std::cout << "Unordered total = " << total_ids2
            << ";  time = " << 1000.0 * ((double) tot_time2) / (double) CLOCKS_PER_SEC
            << " ms." << std::endl;


}
