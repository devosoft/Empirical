#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>
#include <type_traits>

#include "emp/base/vector.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/meta/FunInfo.hpp"

std::string StringDup(std::string base, int dup) {
  std::string out = "";
  for (int i = 0; i < dup; i++) { out += base; }
  return out;
}

TEST_CASE("Test FunInfo", "[meta]")
{
  auto lambda1 = [](){ return 0; };
  auto lambda2 = [](int a, int b, int c){ return a + 2*b + 3*c; };
  auto lambda3 = [](double a, double b){ return a + b*b; };

  std::function<double(double,double,double)> fun4 = lambda2;
  std::function<double(double,double)> fun5 = lambda3;
  std::function<std::string(std::string,int)> fun6 = StringDup;

  std::function<size_t(std::string &)> fun7 = [](std::string & in){ in += '+'; return in.size(); };

  using info0_t = emp::FunInfo<decltype(StringDup)>;
  using info1_t = emp::FunInfo<decltype(lambda1)>;
  using info2_t = emp::FunInfo<decltype(lambda2)>;
  using info3_t = emp::FunInfo<decltype(lambda3)>;
  using info4_t = emp::FunInfo<decltype(fun4)>;
  using info5_t = emp::FunInfo<decltype(fun5)>;
  using info6_t = emp::FunInfo<decltype(fun6)>;
  using info7_t = emp::FunInfo<decltype(fun7)>;

  // Test NumArgs()
  CHECK(info0_t::NumArgs() == 2);
  CHECK(info1_t::NumArgs() == 0);
  CHECK(info2_t::NumArgs() == 3);
  CHECK(info3_t::NumArgs() == 2);
  CHECK(info4_t::NumArgs() == 3);
  CHECK(info5_t::NumArgs() == 2);
  CHECK(info6_t::NumArgs() == 2);
  CHECK(info7_t::NumArgs() == 1);

  // Check return types
  CHECK(std::is_same<info0_t::return_t, std::string>() == true);
  CHECK(std::is_same<info1_t::return_t, int>() == true);
  CHECK(std::is_same<info2_t::return_t, int>() == true);
  CHECK(std::is_same<info3_t::return_t, double>() == true);
  CHECK(std::is_same<info4_t::return_t, double>() == true);
  CHECK(std::is_same<info5_t::return_t, double>() == true);
  CHECK(std::is_same<info6_t::return_t, std::string>() == true);
  CHECK(std::is_same<info7_t::return_t, size_t>() == true);

  // Check argument types
  CHECK(std::is_same<info0_t::arg_t<0>, std::string>() == true);
  CHECK(std::is_same<info0_t::arg_t<1>, int>() == true);
  CHECK(std::is_same<info0_t::arg_t<1>, std::string>() == false);
  CHECK(std::is_same<info0_t::arg_t<1>, char>() == false);

  CHECK(std::is_same<info2_t::arg_t<0>, int>() == true);
  CHECK(std::is_same<info2_t::arg_t<1>, int>() == true);
  CHECK(std::is_same<info2_t::arg_t<2>, int>() == true);
  CHECK(std::is_same<info2_t::arg_t<2>, double>() == false);

  CHECK(std::is_same<info3_t::arg_t<0>, double>() == true);
  CHECK(std::is_same<info3_t::arg_t<1>, double>() == true);

  CHECK(std::is_same<info4_t::arg_t<0>, double>() == true);
  CHECK(std::is_same<info4_t::arg_t<1>, double>() == true);
  CHECK(std::is_same<info4_t::arg_t<2>, double>() == true);

  CHECK(std::is_same<info5_t::arg_t<0>, double>() == true);
  CHECK(std::is_same<info5_t::arg_t<1>, double>() == true);

  CHECK(std::is_same<info6_t::arg_t<0>, std::string>() == true);
  CHECK(std::is_same<info6_t::arg_t<1>, int>() == true);

  CHECK(std::is_same<info7_t::arg_t<0>, std::string &>() == true);
  CHECK(std::is_same<info7_t::arg_t<0>, std::string>() == false); // Must be reference!
  CHECK(std::is_same<info7_t::arg_t<0>, const std::string &>() == false); // Not const!
}
