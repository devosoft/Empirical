/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file FunInfo.cpp
 */

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <sstream>
#include <string>
#include <type_traits>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/meta/FunInfo.hpp"
#include "emp/tools/string_utils.hpp"

std::string fun0(std::string base, int dup) {
  std::string out = "";
  for (int i = 0; i < dup; i++) { out += base; }
  return out;
}

TEST_CASE("Test FunInfo", "[meta]")
{
  auto fun1 = [](){ return 0; };
  auto fun2 = [](int a, int b, int c){ return a + 2*b + 3*c; };
  auto fun3 = [](double a, double b){ return a + b*b; };

  std::function<double(double,double,double)> fun4 = fun2;
  std::function<double(double,double)> fun5 = fun3;
  std::function<std::string(std::string,int)> fun6 = fun0;

  std::function<size_t(std::string &)> fun7 = [](std::string & in){ in += '+'; return in.size(); };

  using info0_t = emp::FunInfo<decltype(fun0)>;
  using info1_t = emp::FunInfo<decltype(fun1)>;
  using info2_t = emp::FunInfo<decltype(fun2)>;
  using info3_t = emp::FunInfo<decltype(fun3)>;
  using info4_t = emp::FunInfo<decltype(fun4)>;
  using info5_t = emp::FunInfo<decltype(fun5)>;
  using info6_t = emp::FunInfo<decltype(fun6)>;
  using info7_t = emp::FunInfo<decltype(fun7)>;

  // Test num_args
  CHECK(info0_t::num_args == 2);
  CHECK(info1_t::num_args == 0);
  CHECK(info2_t::num_args == 3);
  CHECK(info3_t::num_args == 2);
  CHECK(info4_t::num_args == 3);
  CHECK(info5_t::num_args == 2);
  CHECK(info6_t::num_args == 2);
  CHECK(info7_t::num_args == 1);

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

  // Test how functions can be invoked!
  const std::string const_string = "My Unchanging String";
  std::string mut_string = "This String Can Change";

  CHECK(info0_t::InvocableWith("abc", 27) == true);
  CHECK(info0_t::InvocableWith(27, "abc") == false);
  CHECK(info0_t::InvocableWith("abc") == false);
  CHECK(info0_t::InvocableWith("abc", 27, 28) == false);
  CHECK(info0_t::InvocableWith(const_string, 1) == true);
  CHECK(info0_t::InvocableWith(mut_string, 10000) == true);

  CHECK(info0_t::InvocableWith<std::string,int>() == true);
  CHECK(info0_t::InvocableWith<int, std::string>() == false);
  // CHECK(info0_t::InvocableWith("abc") == false);
  // CHECK(info0_t::InvocableWith("abc", 27, 28) == false);
  // CHECK(info0_t::InvocableWith(const_string, 1) == true);
  // CHECK(info0_t::InvocableWith(mut_string, 10000) == true);

  CHECK(info0_t::InvocableWith<std::string>("only_one", 1) == true);

  CHECK(info1_t::InvocableWith() == true);
  CHECK(info1_t::InvocableWith(1) == false);
  CHECK(info1_t::InvocableWith(mut_string) == false);

  CHECK(info2_t::InvocableWith(2, 3, 4) == true);
  CHECK(info2_t::InvocableWith(-2, -3, -4) == true);
  CHECK(info2_t::InvocableWith(2, 3, '4') == true);
  CHECK(info2_t::InvocableWith(2, 3, "4") == false);
  CHECK(info2_t::InvocableWith(2, 3) == false);
  CHECK(info2_t::InvocableWith(2, 3, 4, 5) == false);

  CHECK(info3_t::InvocableWith(2, 3) == true);
  CHECK(info3_t::InvocableWith(1.2, 3.4) == true);

  CHECK(info4_t::InvocableWith(2, 3, 4) == true);
  CHECK(info4_t::InvocableWith(1.2, 3.4, 5.6) == true);

  CHECK(info5_t::InvocableWith(2, 3) == true);
  CHECK(info5_t::InvocableWith(1.2, 3.4) == true);
  CHECK(info5_t::InvocableWith(1.1111111, 3.3333333) == true);

  CHECK(info6_t::InvocableWith("abc", 27) == true);
  CHECK(info6_t::InvocableWith(27, "abc") == false);
  CHECK(info6_t::InvocableWith("abc") == false);
  CHECK(info6_t::InvocableWith("abc", 27, 28) == false);
  CHECK(info6_t::InvocableWith(const_string, 1) == true);
  CHECK(info6_t::InvocableWith(mut_string, 10000) == true);

  CHECK(info7_t::InvocableWith(mut_string) == true);
  CHECK(info7_t::InvocableWith(const_string) == false);
  CHECK(info7_t::InvocableWith("abc") == false);


  // Now try actually running them!
  CHECK(fun0("abc", 6) == "abcabcabcabcabcabc");
  CHECK(fun0(const_string, 2) == const_string + const_string);
  CHECK(fun0(mut_string, 1000).size() == 22000);

  CHECK(fun1() == 0);

  CHECK(fun2(2, 3, 4) == 20);
  CHECK(fun2(-2, -3, -4) == -20);
  CHECK(fun2(2, 3, '4') == 164);

  CHECK(fun3(2, 3) == 11);
  CHECK(fun3(1.2, 3.4) == Approx(12.76));

  CHECK(fun4(2, 3, 4) == 20);
  CHECK(fun4(1.2, 3.4, 5.6) == 22.0);

  CHECK(fun5(2, 3) == 11.0);
  CHECK(fun5(1.2, 3.4) == Approx(12.76));
  CHECK(fun5(1.1111111, 3.3333333) == Approx(12.2222219889));

  CHECK(fun6("abc", 6) == "abcabcabcabcabcabc");
  CHECK(fun6(const_string, 2) == const_string + const_string);
  CHECK(fun6(mut_string, 1000).size() == 22000);

  CHECK(fun7(mut_string) == 23);
  CHECK(mut_string == "This String Can Change+");


  // Test binding the first parameter.
  auto fun0b = emp::BindFirst(fun0, "123");
  CHECK(fun0b(3) == "123123123");
  CHECK(fun0b(10) == "123123123123123123123123123123");

  auto fun0c = emp::BindFirst(fun0, "Test");
  CHECK(fun0c(3) == "TestTestTest");
  CHECK(fun0c(10) == "TestTestTestTestTestTestTestTestTestTest");

  auto fun2a = emp::BindAt<0>(fun2, 10);
  auto fun2b = emp::BindAt<1>(fun2, 10);
  auto fun2c = emp::BindAt<2>(fun2, 10);

  CHECK( fun2a(2,3) == 23 );
  CHECK( fun2b(2,3) == 31 );
  CHECK( fun2c(2,3) == 38 );

  CHECK( fun2a(5,15) == 65 );
  CHECK( fun2b(5,15) == 70 );
  CHECK( fun2c(5,15) == 65 );

  std::string test_str = "Start";
  auto fun7b = emp::BindAt<0>(fun7, test_str);

  CHECK( test_str == "Start" );
  fun7b();
  CHECK( test_str == "Start+" );
  fun7b();
  CHECK( test_str == "Start++" );
  fun7b();
  CHECK( test_str == "Start+++" );


  std::string test_str2 = "Again";
  auto fun7c = emp::BindFirst(fun7, test_str2);

  CHECK( test_str2 == "Again" );
  fun7c();
  CHECK( test_str2 == "Again+" );
  fun7c();
  CHECK( test_str2 == "Again++" );
  fun7c();
  CHECK( test_str2 == "Again+++" );


  // Test function wrapping!
  auto concat = [](std::string in1, std::string in2){ return in1 + in2; };
  CHECK( concat("abc","def") == "abcdef" );

  auto concat_to_int =
    emp::ChangeReturnType(
      concat,
      [](std::string x){ return emp::from_string<int>(x); }
    );

  CHECK( concat_to_int("123","456") == 123456 );

  auto concat_from_int =
    emp::ChangeParameterTypes<int>(
      concat,
      [](int x){ return emp::to_string(x); }
    );

  CHECK( concat_from_int(123,456) == "123456" );

  auto concat_all_int =
    emp::ChangeTypes<int>(
      concat,
      [](std::string x){ return emp::from_string<int>(x); },
      [](int x){ return emp::to_string(x); }
    );

  CHECK( concat_all_int(100+23,456) == 123456 );

  // A slightly more complex conversion.
  auto fancy_dup = emp::ConvertParameterTypes<std::string>(
    fun0,
    [](std::string in_value, auto out_type) {
      return emp::from_string<decltype(out_type)>(in_value);
    }
  );

  CHECK( fancy_dup("double", "2") == "doubledouble" );
  CHECK( fancy_dup("abc", "10") == "abcabcabcabcabcabcabcabcabcabc" );

  auto concat5 = [](std::string a, std::string b, std::string c, std::string d, std::string e) {
    return emp::to_string(a,b,c,d,e);
  };

  CHECK( concat5("a","bcd","e","fghij","kl") == "abcdefghijkl");

  auto concat5a = emp::Bind<0,2,3>(concat5, "[", "]", ": ");

  CHECK( concat5a("Test Status", "Success!") == "[Test Status]: Success!");

  auto concat5b = emp::Bind<1,3,4>(concat5, "bcd", "fghij", "kl");

  CHECK( concat5b("a","e") == "abcdefghijkl" );

  auto concat5c = emp::Bind<0,1,2,3,4>(concat5, "01", "23", "45", "67", "89");

  CHECK( concat5c() == "0123456789" );

  // std::string middle = "X";
  // auto concat5d = emp::Bind<0,1,2,3,4>(concat5, "<<<", "==", middle, "==", ">>>");

  // CHECK( concat5d() == "<<<==X==>>>" );

  // middle = "()";

  // CHECK( concat5d() == "<<<==()==>>>" );

}
