/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/Fraction.hpp"

TEST_CASE("Test Fraction Constructor", "[math]") {
  emp::Fraction frac1;
  REQUIRE(frac1.GetNumerator() == 0);
  REQUIRE(frac1.GetDenominator() == 1);
  frac1.Reduce();
  REQUIRE(frac1.GetNumerator() == 0);
  REQUIRE(frac1.GetDenominator() == 1);

  emp::Fraction frac2(5, 10);
  REQUIRE(frac2.GetNumerator() == 5);
  REQUIRE(frac2.GetDenominator() == 10);
  frac2.Reduce();
  REQUIRE(frac2.GetNumerator() == 1);
  REQUIRE(frac2.GetDenominator() == 2);

  emp::Fraction frac3(0, 10);
  REQUIRE(frac3.GetNumerator() == 0);
  REQUIRE(frac3.GetDenominator() == 10); // Zero numerator
  frac3.Reduce();
  REQUIRE(frac3.GetNumerator() == 0);
  REQUIRE(frac3.GetDenominator() == 1);

  emp::Fraction frac4(10, -5);
  REQUIRE(frac4.GetNumerator() == 10); // Negative handling
  REQUIRE(frac4.GetDenominator() == -5);
  frac4.Reduce();
  REQUIRE(frac4.GetNumerator() == -2);
  REQUIRE(frac4.GetDenominator() == 1);

  emp::Fraction frac5(-10, 5);
  REQUIRE(frac5.GetNumerator() == -10); // Negative handling
  REQUIRE(frac5.GetDenominator() == 5);
  frac5.Reduce();
  REQUIRE(frac5.GetNumerator() == -2);
  REQUIRE(frac5.GetDenominator() == 1);

  emp::Fraction frac6(-10, -5);
  REQUIRE(frac6.GetNumerator() == -10); // Negative handling
  REQUIRE(frac6.GetDenominator() == -5);
  frac6.Reduce();
  REQUIRE(frac6.GetNumerator() == 2);
  REQUIRE(frac6.GetDenominator() == 1);

}

TEST_CASE("Test Zero Denominator", "[math]") {
  emp::Fraction frac(10, 0);
  REQUIRE(frac.GetNumerator() == 10);
  REQUIRE(frac.GetDenominator() == 0);
  frac.Reduce();
  REQUIRE(frac.GetNumerator() == 10);
  REQUIRE(frac.GetDenominator() == 0); // Expect no change for zero denominator
}
