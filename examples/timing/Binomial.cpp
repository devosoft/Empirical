#include <iostream>
#include <ctime>

#include "../../include/emp/math/Distribution.hpp"
#include "../../include/emp/math/Random.hpp"

void TestGeometric(emp::Random & random, const double p, const size_t num_tests=1000) {
  emp::NegativeBinomial dist(p, 1);
  emp::vector<double> results;

  std::clock_t start_time = std::clock();

  size_t total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    total += dist.PickRandom(random);
  }

  std::clock_t tot_time = std::clock() - start_time;
  double result = 1000.0 * ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Negative Binomial with p = " << p << " (and N=1)\n"
            << "  time = " << result << " seconds.\n"
            << "  dist size = " << dist.GetSize() << "\n"
            << "  total = " << total << "\n"
            << std::endl;
}

int main()
{
  size_t num_tests = 1000000;
  emp::Random random;

  TestGeometric(random, 0.9, num_tests);
  TestGeometric(random, 0.5, num_tests);
  TestGeometric(random, 0.1, num_tests);
  TestGeometric(random, 0.01, num_tests);
  TestGeometric(random, 0.001, num_tests);
  TestGeometric(random, 0.0001, num_tests);
}