/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 */

#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/Datum.hpp"


TEST_CASE("Test Datum", "[data]")
{

  emp::Datum d1;
  CHECK(d1.IsDouble() == true);

  emp::Datum d2("123");
  emp::Datum d3(456);
  d1 = "789";
  emp::Datum d4(d1);

  CHECK(d1.IsDouble() == false);
  CHECK(d2.IsDouble() == false);
  CHECK(d3.IsDouble() == true);
  CHECK(d4.IsDouble() == false);

  CHECK(d1 == 789);
  CHECK(d2 == 123);
  CHECK(d3 == 456);
  CHECK(d4 == 789);

  d2 = d2.AsDouble();
  d3 = d3.AsString();
  d4 = d4.AsDouble();

  CHECK(d1.IsString() == true);
  CHECK(d2.IsString() == false);
  CHECK(d3.IsString() == true);
  CHECK(d4.IsString() == false);

  CHECK(d1 == "789");
  CHECK(d2 == "123.000000");
  CHECK(d3 == "456");
  CHECK(d4 == "789.000000");

  // Check string comparisons (d1 and d3)
  CHECK(d1 != "abc");
  CHECK(d3 != "");
  CHECK(d1 < "987");
  CHECK(d3 > "3");
  CHECK(d1 <= "987");
  CHECK(d3 >= "3");
  CHECK(d1 <= "789");
  CHECK(d3 >= "456");

  CHECK(d1 == d1);
  CHECK(d1 >= d1);
  CHECK(d1 <= d1);
  CHECK(d1 != d3);
  CHECK(d1 >= d3);
  CHECK(d1 > d3);
  CHECK(d3 != d1);
  CHECK(d3 <= d1);
  CHECK(d3 < d1);

  // Check number comparisons (d2 and d4)
  CHECK(d2 != 234);
  CHECK(d4 != 678);
  CHECK(d2 <  345);
  CHECK(d4 >  456.7);
  CHECK(d2 <= 789);
  CHECK(d4 >= 3.14);
  CHECK(d2 <= 123);
  CHECK(d4 >= 789);

  CHECK(d2 == d2);
  CHECK(d2 >= d2);
  CHECK(d2 <= d2);
  CHECK(d2 != d4);
  CHECK(d2 <= d4);
  CHECK(d2 < d4);
  CHECK(d4 != d2);
  CHECK(d4 >= d2);
  CHECK(d4 > d2);


  // Mixed comparisons
  CHECK(d1 > d2);
  CHECK(d1 >= d4);
  CHECK(d2 < d3);
  CHECK(d2 != "abc");
  CHECK(d2 != "234");
  CHECK(d2 < "234");
  CHECK(d2 <= "9");
  CHECK(d2 >= "0");
  CHECK(d2 > "0000000");

  CHECK(d3 != 234);
  CHECK(d3 < 789);
  CHECK(d3 <= 678);
  CHECK(d3 >= 234);
  CHECK(d3 > 0);

  double x = d1;
  std::string y = d1;

  CHECK(x == 789);
  CHECK(y == "789");

  x = d2;
  y = d2.AsString();

  CHECK(x == 123.0);
  CHECK(y == "123");

}

TEST_CASE("Test Datum Math", "[data]")
{
  emp::Datum d2(2.0);
  emp::Datum d3(3.0);
  emp::Datum ds("Num:");

  CHECK(d2 + 1.0 == d3);
  CHECK(d2 + d3 == 5.0);
  CHECK(d3 + 3.0 == 6.0);
  CHECK(ds + 1.5 == "Num:1.500000");

  CHECK(d2 * 2.0 == 4.0);
  CHECK(d3 * 3.0 == 9.0);
  CHECK(ds * 4.0 == "Num:Num:Num:Num:");

  CHECK(d2 - 1.0 == 1.0);
  CHECK(d3 - 1.0 == 2.0);

  CHECK(d2 / 2.0 == 1.0);
  CHECK(d3 / 2.0 == 1.5);

  CHECK(d3 % 2.0 == 1.0);

  CHECK(1.0 + d2 == d3);
  CHECK(3.0 + d3 == 6.0);
  CHECK(2.0 * d2 == 4.0);
  CHECK(5.0 - d2 == 3.0);
  CHECK(12.0 / d3 == 4.0);
  CHECK(13.0 % d3 == 1.0);

  d2 += d3;
  CHECK(d2 == 5.0);
  d2 += 2.0;
  CHECK(d2 == 7.0);
  d2 -= 4.0;
  CHECK(d2 == 3.0);
  d2 *= 10.0;
  CHECK(d2 == 30.0);
  d2 /= 6.0;
  CHECK(d2 == 5.0);
  d2 %= 3.0;
  CHECK(d2 == 2.0);

  CHECK(+d2 == 2.0);
  CHECK(-d2 == -2.0);
  CHECK(!d2 == false);

  ds += "string";
  CHECK(ds == "Num:string");

  double dval = d2;
  std::string sval = ds;
  CHECK(dval == 2.0);
  CHECK(sval == "Num:string");
}
