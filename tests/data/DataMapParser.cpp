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

  emp::DataMap dmB(dmA);
  dmB.Get<double>("val1") = 0.125;
  dmB.Get<double>("val2") = 64.25;
  dmB.Get<int>("val3") = 4;
  dmB.Get<char>("char") = '%';

  auto fun = emp::DataMapParser::BuildMathFunction(dmA, "5 * 5");
  CHECK( fun(dmA) == 25.0 );
  CHECK( fun(dmB) == 25.0 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "val1 + val2 + 2*val3");
  CHECK( fun(dmA) == 9.5 );
  CHECK( fun(dmB) == 72.375 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "1.5*val3");
  CHECK( fun(dmA) == 4.5 );
  CHECK( fun(dmB) == 6.0 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "val1/0.5");
  CHECK( fun(dmA) == 3.0 );
  CHECK( fun(dmB) == 0.25 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "1.5*val3 - val1/0.5");
  CHECK( fun(dmA) == 1.5 );
  CHECK( fun(dmB) == 5.75 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "val2/64");
  CHECK( fun(dmA) == 0.03125 );
  CHECK( fun(dmB) == 1.00390625 );

  fun = emp::DataMapParser::BuildMathFunction(dmA, "1.5*val3 - val1/0.5 - val2/64");
  CHECK( fun(dmA) == 1.46875 );
  CHECK( fun(dmB) == 4.74609375 );
}