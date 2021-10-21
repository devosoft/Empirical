#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/DataMapParser.hpp"

TEST_CASE("Test DataMap", "[data]")
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


  emp::DataMapParser parser;

  // Test a bunch of constant math.
  auto fun = parser.BuildMathFunction(dmA, "5.5 + 4");
  CHECK( fun(dmA) == 9.5 );
  CHECK( fun(dmB) == 9.5 );

  fun = parser.BuildMathFunction(dmA, "5.5 - 4");
  CHECK( fun(dmA) == 1.5 );
  CHECK( fun(dmB) == 1.5 );

  fun = parser.BuildMathFunction(dmA, "5.5 * 4");
  CHECK( fun(dmA) == 22.0 );
  CHECK( fun(dmB) == 22.0 );

  fun = parser.BuildMathFunction(dmA, "5.5 / 4");
  CHECK( fun(dmA) == 1.375 );
  CHECK( fun(dmB) == 1.375 );

  fun = parser.BuildMathFunction(dmA, "5.5 % 4");
  CHECK( fun(dmA) == 1.5 );
  CHECK( fun(dmB) == 1.5 );

  fun = parser.BuildMathFunction(dmA, "5.5 ** 4");
  CHECK( fun(dmA) == 915.0625 );
  CHECK( fun(dmB) == 915.0625 );

  fun = parser.BuildMathFunction(dmA, "30.25 ** 0.5");
  CHECK( fun(dmA) == 5.5 );
  CHECK( fun(dmB) == 5.5 );

  fun = parser.BuildMathFunction(dmA, "64 %% 4");
  CHECK( fun(dmA) == 3.0 );
  CHECK( fun(dmB) == 3.0 );

  fun = parser.BuildMathFunction(dmA, "1 + 2 * 2");
  CHECK( fun(dmA) == 5.0 );
  CHECK( fun(dmB) == 5.0 );

  fun = parser.BuildMathFunction(dmA, "(1+2) * 2");
  CHECK( fun(dmA) == 6.0 );
  CHECK( fun(dmB) == 6.0 );

  fun = parser.BuildMathFunction(dmA, "(3*3 + 4**2) ** 0.5");
  CHECK( fun(dmA) == 5.0 );
  CHECK( fun(dmB) == 5.0 );


  // Now, try to use these with variables!

  fun = parser.BuildMathFunction(dmA, "val1 + val2 + 2*val3");
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
  CHECK( fun(dmA) == Approx(1.1447142426) );
  CHECK( fun(dmB) == Approx(0.5) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val2)");
  CHECK( fun(dmA) == Approx(1.4142135624) );
  CHECK( fun(dmB) == Approx(8.0156097709) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val3)");
  CHECK( fun(dmA) == Approx(1.7320508076) );
  CHECK( fun(dmB) == 2.0 );

  fun = parser.BuildMathFunction(dmA, "CBRT(val4)");
  CHECK( fun(dmA) == Approx(6.3496042079) );
  CHECK( fun(dmB) == Approx(10.0793683992) );

  fun = parser.BuildMathFunction(dmA, "SQRT(val3) * SQRT(val2) + CBRT(val1) + CBRT(val4)");
  CHECK( fun(dmA) == Approx(9.9438081932) );
  CHECK( fun(dmB) == Approx(26.610587941) );

  fun = parser.BuildMathFunction(dmA, "LOG(val1) + LOG(val2,9) + LOG2(val3) + LOG10(val4)");
  CHECK( fun(dmA) == Approx(4.7141324511) );
  CHECK( fun(dmB) == Approx(4.8254220245) );

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
  CHECK( fun(dmA) == Approx(0.9974949866) );
  CHECK( fun(dmB) == Approx(0.1246747334) );

  fun = parser.BuildMathFunction(dmA, "COS(val2)");
  CHECK( fun(dmA) == Approx(-0.4161468365) );
  CHECK( fun(dmB) == Approx(0.1520572536) );

  fun = parser.BuildMathFunction(dmA, "TAN(val4 - val3)");
  CHECK( fun(dmA) == Approx(-9.7900600635) );
  CHECK( fun(dmB) == Approx(-1.6194475388) );

  fun = parser.BuildMathFunction(dmA, "SIN(val1) + COS(val2) + TAN(val4 - val3)");
  CHECK( fun(dmA) == Approx(-9.2087119135) );
  CHECK( fun(dmB) == Approx(-1.3427155518) );

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
  CHECK( fun(dmB) == Approx(5.0062460986) );

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
}