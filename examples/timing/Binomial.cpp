#include <iostream>
#include <ctime>

#include "../../include/emp/math/Distribution.hpp"
#include "../../include/emp/math/Random.hpp"

void TestGeometric(emp::Random & random, const double p, const size_t num_tests=1000) {
  std::cout << "---- Geometric Tests: p = " << p << " ----\n";

  ////////- Pre-processed distribution
  emp::NegativeBinomial dist(p, 1);

  std::clock_t start_time = std::clock();

  double total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    total += dist.PickRandom(random);
  }

  std::clock_t tot_time = std::clock() - start_time;
  double result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Negative Binomial Distribution with p = " << p << " (and N=1)\n"
            << "  time = " << result << " seconds.\n"
            << "  dist size = " << dist.GetSize() << "\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;

  ////////- Random call (no pre-process)
  start_time = std::clock();

  total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    total += random.GetGeometric(p);
  }

  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "random.GetGeometric(p) with p = " << p << "\n"
            << "  time = " << result << " seconds.\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;

}

int main()
{
  size_t num_tests = 10000000;
  emp::Random random;

  TestGeometric(random, 0.9, num_tests);
  TestGeometric(random, 0.5, num_tests);
  TestGeometric(random, 0.1, num_tests);
  TestGeometric(random, 0.01, num_tests);
  TestGeometric(random, 0.001, num_tests);
  TestGeometric(random, 0.0001, num_tests);
}