#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>

#include "emp/base/vector.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/meta/FunInfo.hpp"

std::string StringDup(std::string base, int dup) {
  std::string out = "";
  for (int i =0; i < dup; i++) { out += base; }
  return out;
}

TEST_CASE("Test FunInfo", "[meta]")
{
  auto lambda1 = [](){ return 0; };
  auto lambda2 = [](int a, int b, int c){ return a + 2*b + 3*c; };
  auto lambda3 = [](double a, double b){ return a + b*b; };

  std::function<double(double,double,double)> fun4 = lambda2;
  std::function<double(double,double)> fun5 = lambda3;

  using info0 = emp::FunInfo<decltype(StringDup)>;
  using info1 = emp::FunInfo<decltype(lambda1)>;
  using info2 = emp::FunInfo<decltype(lambda2)>;
  using info3 = emp::FunInfo<decltype(lambda3)>;
  using info4 = emp::FunInfo<decltype(fun4)>;
  using info5 = emp::FunInfo<decltype(fun5)>;

  // Test NumArgs()
  REQUIRE(info1::NumArgs() == 0);
}
