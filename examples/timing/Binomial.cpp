#include <ctime>
#include <iostream>

#include "../../include/emp/math/Distribution.hpp"
#include "../../include/emp/math/Random.hpp"
#include "../../include/emp/tools/string_utils.hpp"

void TestGeometric(emp::Random & random, const double p, const size_t num_tests=1000000) {
  std::cout << emp::ANSI_GreenBG() << emp::ANSI_Black()
            << "---- Geometric Tests: p = " << p << " ----"
            << emp::ANSI_Reset()
            << std::endl;

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
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
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
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;

}

void TestNegBinomial(
  emp::Random & random,
  const double p,
  const size_t N,
  const size_t num_tests=1000000)
{
  std::cout << emp::ANSI_BrightBlueBG()
            << "---- Negative Binomial Tests: p = " << p << " ; N = " << N << " ----"
            << emp::ANSI_Reset()
            << std::endl;

  ////////- Pre-processed distribution
  emp::NegativeBinomial dist(p, N);

  std::clock_t start_time = std::clock();

  double total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    total += dist.PickRandom(random);
  }

  std::clock_t tot_time = std::clock() - start_time;
  double result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Negative Binomial Distribution with p = " << p << " and N = " << N << "\n"
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
            << "  dist size = " << dist.GetSize() << "\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;

  ////////- Random call (no pre-process)
  start_time = std::clock();

  total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    for (size_t n = 0; n < N; ++n) {
      total += random.GetGeometric(p);
    }
  }

  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "N = " << N << " calls to random.GetGeometric(p) with p = " << p << "\n"
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;
}

void TestBinomial(
  emp::Random & random,
  const double p,
  const size_t N,
  const size_t num_tests=1000000)
{
  std::cout << emp::ANSI_MagentaBG()
            << "---- Binomial Tests: p = " << p << " ; N = " << N << " ----"
            << emp::ANSI_Reset()
            << std::endl;

  ////////- Pre-processed distribution
  emp::Binomial dist(p, N);

  std::clock_t start_time = std::clock();

  double total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    total += dist.PickRandom(random);
  }

  std::clock_t tot_time = std::clock() - start_time;
  double result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "Binomial Distribution with p = " << p << " and N = " << N << "\n"
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
            << "  dist size = " << dist.GetSize() << "\n"
            << "  average = " << (total / num_tests) << "\n"
            << std::endl;

  ////////- Random call (no pre-process)
  start_time = std::clock();

  total = 0;
  for (size_t i = 0; i < num_tests; i++) {
    size_t pos = 0;
    while( (pos += random.GetGeometric(p)) < N ) {
      total++;
    }
  }

  tot_time = std::clock() - start_time;
  result = ((double) tot_time) / (double) CLOCKS_PER_SEC;

  std::cout << "N = " << N << " calls to random.GetGeometric(p) with p = " << p << "\n"
            << "  time = " << emp::ANSI_Bold() << result << " seconds." << emp::ANSI_NoBold() << "\n"
            << "  average = " << (total / num_tests) << "\n"
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

  TestNegBinomial(random, 0.9, 10, num_tests);
  TestNegBinomial(random, 0.5, 10, num_tests);
  TestNegBinomial(random, 0.1, 10, num_tests);
  TestNegBinomial(random, 0.01, 10, num_tests);
  TestNegBinomial(random, 0.001, 10, num_tests);
  TestNegBinomial(random, 0.0001, 10, num_tests);

  TestNegBinomial(random, 0.9, 100, num_tests);
  TestNegBinomial(random, 0.5, 100, num_tests);
  TestNegBinomial(random, 0.1, 100, num_tests);
  TestNegBinomial(random, 0.01, 100, num_tests);
  TestNegBinomial(random, 0.001, 100, num_tests);
  TestNegBinomial(random, 0.0001, 100, num_tests);

  TestBinomial(random, 0.9, 100, num_tests);
  TestBinomial(random, 0.5, 100, num_tests);
  TestBinomial(random, 0.1, 100, num_tests);
  TestBinomial(random, 0.01, 100, num_tests);
  TestBinomial(random, 0.001, 100, num_tests);
  TestBinomial(random, 0.0001, 100, num_tests);

  TestBinomial(random, 0.9, 1000, num_tests);
  TestBinomial(random, 0.5, 1000, num_tests);
  TestBinomial(random, 0.1, 1000, num_tests);
  TestBinomial(random, 0.01, 1000, num_tests);
  TestBinomial(random, 0.001, 1000, num_tests);
  TestBinomial(random, 0.0001, 1000, num_tests);
}
