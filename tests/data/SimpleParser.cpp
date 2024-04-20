/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022
 *
 *  @file SimpleParser.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <map>
#include <unordered_map>

#include "emp/data/SimpleParser.hpp"

TEST_CASE("Test DataMap in SimpleParser", "[data]")
{
  emp::DataMap dmA;
  dmA.AddVar<double>("val1", 1.5);
  dmA.AddVar<double>("val2", 2.0);
  dmA.AddVar<int>("val3", 3);
  dmA.AddVar<char>("char", 'A');
  dmA.AddVar<double>("val4", 256.0);

  emp::DataMap dmB(dmA);
  dmB.Get<double>("val1") = 0.125;
  dmB.Get<double>("val2") = 64.25;
  dmB.Get<int>("val3") = 4;
  dmB.Get<char>("char") = '%';
  dmB.Get<double>("val4") = 1024.0;

  //         val1  val2  val3  val4
  // dmA:     1.5     2     3   256
  // dmB    0.125 64.25     4  1024


  emp::SimpleParser parser;

  // Test a bunch of constant math.
  CHECK( parser.RunMathFunction(dmA, "(0 || 0)") == 0.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 || 1) + (1 || 0) + (1 || 1)") == 3.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 && 1) + (1 && 0) + (1 && 1)") == 1.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 == 0) + (1 == 1.000000001) + (1.1 == 1.2)") == 1.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~== 0) + (1 ~== 1.000000001) + (1.1 ~== 1.2)") == 2.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 != 0) + (1 != 1.000000001) + (1.1 != 1.2)") == 2.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~!= 0) + (1 ~!= 1.000000001) + (1.1 ~!= 1.2)") == 1.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 < 0) + (1 < 1.000000001) + (1.1 < 1.2)") == 2.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~< 0) + (1 ~< 1.000000001) + (1.1 ~< 1.2)") == 1.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 <= 0) + (1 <= 1.000000001) + (1.1 <= 1.2)") == 3.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~<= 0) + (1 ~<= 1.000000001) + (1.1 ~<= 1.2)") == 3.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 > 0) + (1 > 0.999999999) + (1.3 > 1.2)") == 2.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~> 0) + (1 ~> 0.999999999) + (1.3 ~> 1.2)") == 1.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 >= 0) + (1 >= 1.000000001) + (1.3 >= 1.2)") == 2.0 );
  CHECK( parser.RunMathFunction(dmA, "(0 ~>= 0) + (1 ~>= 1.000000001) + (1.3 ~>= 1.2)") == 3.0 );
  CHECK( parser.RunMathFunction(dmA, "5.5 + 4") == 9.5 );
  CHECK( parser.RunMathFunction(dmA, "5.5 - 4") == 1.5 );
  CHECK( parser.RunMathFunction(dmA, "5.5 * 4") == 22.0 );
  CHECK( parser.RunMathFunction(dmA, "5.5 / 4") == 1.375 );
  CHECK( parser.RunMathFunction(dmA, "5.5 % 4") == 1.5 );
  CHECK( parser.RunMathFunction(dmA, "5.5 ** 4") == 915.0625 );
  CHECK( parser.RunMathFunction(dmA, "30.25 ** 0.5") == 5.5 );
  CHECK( parser.RunMathFunction(dmA, "64 %% 4") == 3.0 );
  CHECK( parser.RunMathFunction(dmA, "1 + 2 * 2") == 5.0 );
  CHECK( parser.RunMathFunction(dmA, "(1+2) * 2") == 6.0 );
  CHECK( parser.RunMathFunction(dmA, "(3*3 + 4**2) ** 0.5") == 5.0 );

  // Test with external variables.
  CHECK( parser.RunMathFunction(dmA, "$0 + $1 * $2", 1.4, 2, 7.1) == 15.6 );

  // Now, try to use these with DataMap variables!

  std::function<emp::Datum(const emp::DataMap &)> fun =
    parser.BuildMathFunction(dmA, "val1 + val2 + 2*val3");
  CHECK( fun(dmA) == 9.5 );
  CHECK( fun(dmB) == 72.375 );

  std::set<std::string> names_used = parser.GetNamesUsed();
  CHECK( names_used.size() == 3 );
  CHECK( emp::Has(names_used, "val1") );
  CHECK( emp::Has(names_used, "val2") );
  CHECK( emp::Has(names_used, "val3") );
  CHECK( !emp::Has(names_used, "val4") );

  fun = parser.BuildMathFunction(dmA, "1.5*val3");
  CHECK( fun(dmA) == 4.5 );
  CHECK( fun(dmB) == 6.0 );

  names_used = parser.GetNamesUsed();
  CHECK( names_used.size() == 1 );
  CHECK( !emp::Has(names_used, "val1") );
  CHECK( !emp::Has(names_used, "val2") );
  CHECK( emp::Has(names_used, "val3") );
  CHECK( !emp::Has(names_used, "val4") );

  fun = parser.BuildMathFunction(dmA, "val1/0.5");
  CHECK( fun(dmA) == 3.0 );
  CHECK( fun(dmB) == 0.25 );

  fun = parser.BuildMathFunction(dmA, "1.5*val3 - val1/0.5");
  CHECK( fun(dmA) == 1.5 );
  CHECK( fun(dmB) == 5.75 );

  fun = parser.BuildMathFunction(dmA, "val2/64");
  CHECK( fun(dmA) == 0.03125 );
  CHECK( fun(dmB) == 1.00390625 );

  fun = parser.BuildMathFunction(dmA, "1.5*val3 - val1/0.5 - val2/64");
  CHECK( fun(dmA) == 1.46875 );
  CHECK( fun(dmB) == 4.74609375 );

  // Pairs of variables with each operation

  fun = parser.BuildMathFunction(dmA, "val1*val2");
  CHECK( fun(dmA) == 3.0 );
  CHECK( fun(dmB) == 8.03125 );

  fun = parser.BuildMathFunction(dmA, "val3/val1");
  CHECK( fun(dmA) == 2.0 );
  CHECK( fun(dmB) == 32.0 );

  fun = parser.BuildMathFunction(dmA, "val2%val1");
  CHECK( fun(dmA) == 0.5 );
  CHECK( fun(dmB) == 0.0 );

  fun = parser.BuildMathFunction(dmA, "val1**val3");
  CHECK( fun(dmA) == 3.375 );
  CHECK( fun(dmB) == 0.000244140625 );

  fun = parser.BuildMathFunction(dmA, "val4%%(val3*2-4)");
  CHECK( fun(dmA) == 8.0 );
  CHECK( fun(dmB) == 5.0 );

  // ...and all together now...
  fun = parser.BuildMathFunction(dmA, "val1*val2 + val3/val1 + val2%val1 + val1**val3 - val4%%(val3*2-4)");
  CHECK( fun(dmA) == 0.875 );
  CHECK( fun(dmB) == 35.031494140625 );

  names_used = parser.GetNamesUsed();
  CHECK( names_used.size() == 4 );
  CHECK( emp::Has(names_used, "val1") );
  CHECK( emp::Has(names_used, "val2") );
  CHECK( emp::Has(names_used, "val3") );
  CHECK( emp::Has(names_used, "val4") );

  // Using functions.
  fun = parser.BuildMathFunction(dmA, "EXP(val1,val3)");
  CHECK( fun(dmA) == 3.375 );
  CHECK( fun(dmB) == 0.000244140625 );

  fun = parser.BuildMathFunction(dmA, "LOG(val4, val3*2-4)");
  CHECK( fun(dmA) == 8.0 );
  CHECK( fun(dmB) == 5.0 );

  fun = parser.BuildMathFunction(dmA, "ABS(val3 - val2)");
  CHECK( fun(dmA) == 1.0 );
  CHECK( fun(dmB) == 60.25 );

  fun = parser.BuildMathFunction(dmA, "CBRT(val1)");
  CHECK( fun(dmA).AsDouble() == Approx(1.1447142426) );
  CHECK( fun(dmB).AsDouble() == Approx(0.5) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val2)");
  CHECK( fun(dmA).AsDouble() == Approx(1.4142135624) );
  CHECK( fun(dmB).AsDouble() == Approx(8.0156097709) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val3)");
  CHECK( fun(dmA).AsDouble() == Approx(1.7320508076) );
  CHECK( fun(dmB) == 2.0 );

  fun = parser.BuildMathFunction(dmA, "CBRT(val4)");
  CHECK( fun(dmA).AsDouble() == Approx(6.3496042079) );
  CHECK( fun(dmB).AsDouble() == Approx(10.0793683992) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val3) * SQRT(val2) + CBRT(val1) + CBRT(val4)");
  CHECK( fun(dmA).AsDouble() == Approx(9.9438081932) );
  CHECK( fun(dmB).AsDouble() == Approx(26.610587941) );

  fun = parser.BuildMathFunction(dmA, "LOG(val1) + LOG(val2,9) + LOG2(val3) + LOG10(val4)");
  CHECK( fun(dmA).AsDouble() == Approx(4.7141324511) );
  CHECK( fun(dmB).AsDouble() == Approx(4.8254220245) );

  names_used = parser.GetNamesUsed();
  CHECK( names_used.size() == 4 );
  CHECK( emp::Has(names_used, "val1") );
  CHECK( emp::Has(names_used, "val2") );
  CHECK( emp::Has(names_used, "val3") );
  CHECK( emp::Has(names_used, "val4") );
  CHECK( !emp::Has(names_used, "LOG") );
  CHECK( !emp::Has(names_used, "LOG2") );
  CHECK( !emp::Has(names_used, "LOG10") );

  fun = parser.BuildMathFunction(dmA, "SIN(val1)");
  CHECK( fun(dmA).AsDouble() == Approx(0.9974949866) );
  CHECK( fun(dmB).AsDouble() == Approx(0.1246747334) );

  fun = parser.BuildMathFunction(dmA, "COS(val2)");
  CHECK( fun(dmA).AsDouble() == Approx(-0.4161468365) );
  CHECK( fun(dmB).AsDouble() == Approx(0.1520572536) );

  fun = parser.BuildMathFunction(dmA, "TAN(val4 - val3)");
  CHECK( fun(dmA).AsDouble() == Approx(-9.7900600635) );
  CHECK( fun(dmB).AsDouble() == Approx(-1.6194475388) );

  fun = parser.BuildMathFunction(dmA, "SIN(val1) + COS(val2) + TAN(val4 - val3)");
  CHECK( fun(dmA).AsDouble() == Approx(-9.2087119135) );
  CHECK( fun(dmB).AsDouble() == Approx(-1.3427155518) );

  fun = parser.BuildMathFunction(dmA, "CEIL(SIN(val1))");
  CHECK( fun(dmA) == 1.0 );
  CHECK( fun(dmB) == 1.0 );

  names_used = parser.GetNamesUsed();
  CHECK( names_used.size() == 1 );
  CHECK( emp::Has(names_used, "val1") );
  CHECK( !emp::Has(names_used, "val2") );
  CHECK( !emp::Has(names_used, "val3") );
  CHECK( !emp::Has(names_used, "val4") );
  CHECK( !emp::Has(names_used, "CEIL") );
  CHECK( !emp::Has(names_used, "SIN") );

  fun = parser.BuildMathFunction(dmA, "FLOOR(COS(val2))");
  CHECK( fun(dmA) == -1.0 );
  CHECK( fun(dmB) == 0.0 );

  fun = parser.BuildMathFunction(dmA, "ROUND(TAN(val4 - val3))");
  CHECK( fun(dmA) == -10.0 );
  CHECK( fun(dmB) == -2.0 );

  fun = parser.BuildMathFunction(dmA, "HYPOT(2*val1, val3+1)");
  CHECK( fun(dmA) == 5.0 );
  CHECK( fun(dmB).AsDouble() == Approx(5.0062460986) );

  fun = parser.BuildMathFunction(dmA, "MIN(val2, val3)");
  CHECK( fun(dmA) == 2.0 );
  CHECK( fun(dmB) == 4.0 );

  fun = parser.BuildMathFunction(dmA, "MAX(val1*200, val4)");
  CHECK( fun(dmA) == 300.0 );
  CHECK( fun(dmB) == 1024.0 );

  fun = parser.BuildMathFunction(dmA, "IF(val1 > 1, val3, val4)");
  CHECK( fun(dmA) == 3.0 );
  CHECK( fun(dmB) == 1024.0 );

  fun = parser.BuildMathFunction(dmA, "CLAMP(val1, 0, 1)");
  CHECK( fun(dmA) == 1.0 );
  CHECK( fun(dmB) == 0.125 );

  fun = parser.BuildMathFunction(dmA, "CLAMP(val4, 500, 600)");
  CHECK( fun(dmA) == 500.0 );
  CHECK( fun(dmB) == 600.0 );

  fun = parser.BuildMathFunction(dmA, "TO_SCALE(val1, 1000, 2000)");
  CHECK( fun(dmA) == 2500.0 );
  CHECK( fun(dmB) == 1125.0 );

  fun = parser.BuildMathFunction(dmA, "FROM_SCALE(val3, 1.5, 11.5)");
  CHECK( fun(dmA) == 0.15 );
  CHECK( fun(dmB) == 0.25 );


  // Test with more extra values.
  double multiple = 2.0;
  std::string expression = "(val1 + val2 + 2*val3) * $0";

  fun = parser.BuildMathFunction(dmA, expression, multiple);
  CHECK( fun(dmA) == 19 );
  CHECK( fun(dmB) == 144.75 );

  multiple = 1.0;
  fun = parser.BuildMathFunction(dmA, expression, multiple);
  CHECK( fun(dmA) == 9.5 );
  CHECK( fun(dmB) == 72.375 );


  // Test with string concatenation for '+'
  emp::DataMap dmC;
  dmC.AddVar<std::string>("val1", "abc");
  dmC.AddVar<std::string>("val2", "def");
  fun = parser.BuildMathFunction(dmC, "val1 + val2");
  CHECK(fun(dmC).AsString() == "abcdef");
}

TEST_CASE("Test std::map in SimpleParser", "[data]")
{
  std::map<std::string, double> var_map;
  var_map["x"] = 5;
  var_map["y"] = 10;

  emp::SimpleParser parser;
  auto fun = parser.BuildMathFunction(var_map, "11*x + y*y");

  CHECK(fun(var_map) == 155.0);

  var_map["x"] = 3.5;
  var_map["y"] = 5;

  CHECK(fun(var_map) == 63.5);
}

TEST_CASE("Test emp::ra_map in SimpleParser", "[data]")
{
  emp::ra_map<std::string, double> var_map;
  var_map["x"] = 5;
  var_map["y"] = 10;

  emp::SimpleParser parser;
  auto fun = parser.BuildMathFunction(var_map, "11*x + y*y");

  CHECK(fun(var_map) == 155.0);

  var_map["x"] = 3.5;
  var_map["y"] = 5;

  CHECK(fun(var_map) == 63.5);
}
