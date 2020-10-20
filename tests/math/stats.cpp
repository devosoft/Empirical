#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/stats.hpp"
#include "emp/datastructs/vector_utils.hpp"

#include <sstream>
#include <iostream>
#include <deque>


TEST_CASE("Test stats", "[math]") {
  emp::vector<int> vec1({1,2,1,1,2,3});
  double i1 = 1;
  double i2 = 1;
  double i3 = 1;
  double i4 = 2;

  emp::vector<double*> vec2({&i1, &i2, &i3, &i4});

  std::deque<double> deque1({5,4,3,5,4,6});

  REQUIRE(emp::ShannonEntropy(vec1) == Approx(1.459324));
  REQUIRE(emp::ShannonEntropy(vec2) == Approx(0.81128));
  REQUIRE(emp::ShannonEntropy(deque1) == Approx(1.918648));

  REQUIRE(emp::Variance(vec1) == Approx(0.66666).epsilon(0.001));
  REQUIRE(emp::Variance(vec2) == Approx(0.25).epsilon(0.001));
  REQUIRE(emp::Variance(deque1) == Approx(1.1).epsilon(0.001));

  REQUIRE(emp::StandardDeviation(vec1) == Approx(0.81649).epsilon(0.001));
  REQUIRE(emp::StandardDeviation(vec2) == Approx(0.5).epsilon(0.001));
  REQUIRE(emp::StandardDeviation(deque1) == Approx(1.0488).epsilon(0.001));

  REQUIRE(emp::StandardError(vec1) == Approx(0.3333).epsilon(0.001));
  REQUIRE(emp::StandardError(vec2) == Approx(0.25).epsilon(0.001));
  REQUIRE(emp::StandardError(deque1) == Approx(0.4281).epsilon(0.001));

  REQUIRE(emp::Sum(vec1) == 10);
  REQUIRE(emp::Sum(vec2) == 5);
  REQUIRE(emp::Sum(deque1) == 27);

  REQUIRE(emp::UniqueCount(vec1) == 3);
  REQUIRE(emp::UniqueCount(vec2) == 2);
  REQUIRE(emp::UniqueCount(deque1) == 4);

  REQUIRE(emp::Mean(vec1) == Approx(1.6666666666667));
  REQUIRE(emp::Mean(vec2) == Approx(1.25));
  REQUIRE(emp::Mean(deque1) == 4.5);

  std::function<int(int)> invert = [](int i){return i*-1;};

  REQUIRE(emp::MaxResult(invert, vec1) == -1);
  REQUIRE(emp::MinResult(invert, vec1) == -3);
  REQUIRE(emp::MeanResult(invert, vec1) == Approx(-1.666666667));
  REQUIRE(emp::ApplyFunction(invert, vec1) == emp::vector<int>({-1,-2,-1,-1,-2,-3}));

}
