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

  fun = parser.BuildMathFunction(dmA, "1.5*val3");
  CHECK( fun(dmA) == 4.5 );
  CHECK( fun(dmB) == 6.0 );

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

  // 0.125, 64.25, 4, 1024.0;


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

}