#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/DataMapParser.hpp"

TEST_CASE("Test DataMap", "[data]")
{
  emp::DataMap dmA;
  dmA.AddVar<double>("val1", 1.234);
  dmA.AddVar<double>("val2", 2.0);
  dmA.AddVar<int>("val3", 3);
  dmA.AddVar<char>("char", 'A');

  emp::DataMap dmB(dmA);
  dmB.Get<double>("val1") = 4.44;
  dmB.Get<double>("val2") = 55.5;
  dmB.Get<int>("val3") = 100;
  dmB.Get<char>("char") = '%';

  auto fun1 = emp::DataMapParser::BuildMathFunction(dmA, "5 * 5");
  std::cout << "result1A = " << fun1(dmA) << std::endl;
  std::cout << "result1B = " << fun1(dmB) << std::endl;

  auto fun2 = emp::DataMapParser::BuildMathFunction(dmA, "val1 + val2 + 2*val3");
  std::cout << "result2A = " << fun2(dmA) << std::endl;
  std::cout << "result2B = " << fun2(dmB) << std::endl;
}