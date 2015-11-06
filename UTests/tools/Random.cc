// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/Random.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::Random rng;

  // Test GetDouble with the law of large numbers.
  std::vector<int> val_counts(10);
  for (int i = 0; i < (int) val_counts.size(); i++) val_counts[i] = 0;

  const int num_tests = 100000;
  const double min_value = 2.5;
  const double max_value = 8.7;
  double total = 0.0;
  for (int i = 0; i < num_tests; i++) {
    const double cur_value = rng.GetDouble(min_value, max_value);
    total += cur_value;
    val_counts[(int) cur_value]++;
  }

  {
    const double expected_mean = (min_value + max_value) / 2.0;
    const double min_threshold = (expected_mean*0.997);
    const double max_threshold = (expected_mean*1.004);
    double mean_value = total/(double) num_tests;
    
    if (verbose == true) {
      std::cout << "DOUBLE Total=" << total
                << "  Mean = " << mean_value
                << "  Expected Mean = " << expected_mean
                << " [min=" << min_threshold
                << ", max=" << max_threshold
                << "]" << std::endl;
      for (int i = 0; i < (int) val_counts.size(); i++) {
        std::cout << i << ":" << ((double) val_counts[i])/(double)num_tests  << "  ";
      }
      std::cout << std::endl;
    }

    emp_assert(mean_value > min_threshold && mean_value < max_threshold);
  }

  // Test GetInt
  for (int i = 0; i < (int) val_counts.size(); i++) val_counts[i] = 0;
  total = 0.0;

  for (int i = 0; i < num_tests; i++) {
    const int cur_value = rng.GetInt((int) min_value, (int) max_value);
    total += cur_value;
    val_counts[cur_value]++;
  }

  {
    const double expected_mean = (double) (((int) min_value) + ((int) max_value) - 1) / 2.0;
    const double min_threshold = (expected_mean*0.997);
    const double max_threshold = (expected_mean*1.004);
    double mean_value = total/(double) num_tests;
    
    if (verbose == true) {
      std::cout << std::endl
                << "INT Total=" << total
                << "  Mean = " << mean_value
                << "  Expected Mean = " << expected_mean
                << " [min=" << min_threshold
                << ", max=" << max_threshold
                << "]" << std::endl;
      for (int i = 0; i < (int) val_counts.size(); i++) {
        std::cout << i << ":" << ((double) val_counts[i])/(double)num_tests  << "  ";
      }
      std::cout << std::endl;
    }

    emp_assert(mean_value > min_threshold && mean_value < max_threshold);
  }


  // Test P
  double flip_prob = 0.56789;
  int hit_count = 0;
  for (int i = 0; i < num_tests; i++) {
    if (rng.P(flip_prob)) hit_count++;
  }

  double actual_prob = ((double) hit_count) / (double) num_tests;

  if (verbose == true) {
    std::cout << std::endl
              << "Target prob = " << flip_prob
              << "   actual_prob = " << actual_prob
              << std::endl;
  }
   
  emp_assert(actual_prob < flip_prob + 0.005);
  emp_assert(actual_prob > flip_prob - 0.005);
 
  return 0;
}
