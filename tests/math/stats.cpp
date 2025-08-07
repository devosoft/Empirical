/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2021-2025
*/
/**
 *  @file
 */

#include <deque>
#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/vector_utils.hpp"
#include "emp/math/stats.hpp"

TEST_CASE("Test stats", "[math]") {
  emp::vector<int> vec1({1,2,1,1,2,3});
  double i1 = 1;
  double i2 = 1;
  double i3 = 1;
  double i4 = 2;

  emp::vector<double*> vec2({&i1, &i2, &i3, &i4});

  std::deque<double> deque1({5,4,3,5,4,6});

  CHECK(emp::ShannonEntropy(vec1) == Approx(1.459158));
  CHECK(emp::ShannonEntropy(vec2) == Approx(0.81128));
  CHECK(emp::ShannonEntropy(deque1) == Approx(1.918296));

  CHECK(emp::Variance(vec1) == Approx(0.66666).epsilon(0.001));
  CHECK(emp::Variance(vec2) == Approx(0.25).epsilon(0.001));
  CHECK(emp::Variance(deque1) == Approx(1.1).epsilon(0.001));

  CHECK(emp::StandardDeviation(vec1) == Approx(0.81649).epsilon(0.001));
  CHECK(emp::StandardDeviation(vec2) == Approx(0.5).epsilon(0.001));
  CHECK(emp::StandardDeviation(deque1) == Approx(1.0488).epsilon(0.001));

  CHECK(emp::StandardError(vec1) == Approx(0.3333).epsilon(0.001));
  CHECK(emp::StandardError(vec2) == Approx(0.25).epsilon(0.001));
  CHECK(emp::StandardError(deque1) == Approx(0.4281).epsilon(0.001));

  CHECK(emp::Sum(vec1) == 10);
  CHECK(emp::Sum(vec2) == 5);
  CHECK(emp::Sum(deque1) == 27);

  CHECK(emp::UniqueCount(vec1) == 3);
  CHECK(emp::UniqueCount(vec2) == 2);
  CHECK(emp::UniqueCount(deque1) == 4);

  CHECK(emp::Mean(vec1) == Approx(1.6666666666667));
  CHECK(emp::Mean(vec2) == Approx(1.25));
  CHECK(emp::Mean(deque1) == 4.5);

  std::function<int(int)> invert = [](int i){return i*-1;};

  CHECK(emp::MaxResult(invert, vec1) == -1);
  CHECK(emp::MinResult(invert, vec1) == -3);
  CHECK(emp::MeanResult(invert, vec1) == Approx(-1.666666667));
  CHECK(emp::ApplyFunction(invert, vec1) == emp::vector<int>({-1,-2,-1,-1,-2,-3}));

}
