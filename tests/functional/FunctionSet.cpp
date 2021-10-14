/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file FunctionSet.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/functional/FunctionSet.hpp"

// should migrate these inside the test case, probably

// utility funcs for FunctionSet test case
int global_var1;
int global_var2;
int global_var3;
int global_var4;

void fun1(int in1, int in2) { global_var1 = in1 + in2; }
void fun2(int in1, int in2) { global_var2 = in1 - in2; }
void fun3(int in1, int in2) { global_var3 = in1 * in2; }
void fun4(int in1, int in2) { global_var4 = in1 / in2; }

double fun5(double input) { return input; }
double fun6(double input) { return input * input; }
double fun7(double input) { return input * input * input; }

TEST_CASE("Test FunctionSet", "[functional]")
{
  // TEST 1: Functions with VOID returns.
  emp::FunctionSet<void(int,int)> fun_set;
  fun_set.Add(&fun1);
  fun_set.Add(&fun2);
  fun_set.Add(&fun3);
  fun_set.Add(&fun4);

  fun_set.Run(10, 2);

  REQUIRE(global_var1 == 12);
  REQUIRE(global_var2 == 8);
  REQUIRE(global_var3 == 20);
  REQUIRE(global_var4 == 5);

  fun_set.Remove(2);           // What if we remove a function

  fun_set.Run(20, 5);

  REQUIRE(global_var1 == 25);
  REQUIRE(global_var2 == 15);
  REQUIRE(global_var3 == 20);  // Same as last time!!
  REQUIRE(global_var4 == 4);

  // Test 2: Functions with non-void returns.
  emp::FunctionSet<double(double)> fun_set2;
  fun_set2.Add(&fun5);
  fun_set2.Add(&fun6);
  fun_set2.Add(&fun7);

  emp::vector<double> out_vals;

  out_vals = fun_set2.Run(10.0);

  REQUIRE(out_vals[0] == 10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == 1000.0);

  out_vals = fun_set2.Run(-10.0);

  REQUIRE(out_vals[0] == -10.0);
  REQUIRE(out_vals[1] == 100.0);
  REQUIRE(out_vals[2] == -1000.0);

  out_vals = fun_set2.Run(0.1);

  REQUIRE(out_vals[0] == 0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] < 0.00100001);
  REQUIRE(out_vals[2] > 0.00099999);

  out_vals = fun_set2.Run(-0.1);

  REQUIRE(out_vals[0] == -0.1);
  REQUIRE(out_vals[1] < 0.01000001);
  REQUIRE(out_vals[1] > 0.00999999);
  REQUIRE(out_vals[2] > -0.00100001);
  REQUIRE(out_vals[2] < -0.00099999);


  // TEST 3: Running functions and auto-processing return values.
  REQUIRE(fun_set2.FindMax(10.0) == 1000.0);
  REQUIRE(fun_set2.FindMax(-10.0) == 100.0);
  REQUIRE(fun_set2.FindMax(0.1) == 0.1);
  REQUIRE(fun_set2.FindMax(-0.1) < 0.0100000001);
  REQUIRE(fun_set2.FindMax(-0.1) > 0.0099999999);

  REQUIRE(fun_set2.FindMin(10.0) == 10.0);
  REQUIRE(fun_set2.FindMin(-10.0) == -1000.0);
  REQUIRE(fun_set2.FindMin(0.1) < 0.0010000001);
  REQUIRE(fun_set2.FindMin(0.1) > 0.0009999999);
  REQUIRE(fun_set2.FindMin(-0.1) == -0.1);

  REQUIRE(fun_set2.FindSum(10.0) == 1110.0);
  REQUIRE(fun_set2.FindSum(-10.0) == -910.0);
  REQUIRE(fun_set2.FindSum(0.1) < 0.1110000001);
  REQUIRE(fun_set2.FindSum(0.1) > 0.1109999999);
  REQUIRE(fun_set2.FindSum(-0.1) == -0.091);


  // TEST 4: Manually-entered aggregation function.
  std::function<double(double,double)> test_fun =
    [](double i1, double i2){ return std::max(i1,i2); };

  REQUIRE(fun_set2.Run(10.0, test_fun) == 1000.0);
  REQUIRE(fun_set2.Run(-10.0, test_fun) == 100.0);
  REQUIRE(fun_set2.Run(0.1, test_fun) == 0.1);
  REQUIRE(fun_set2.Run(-0.1, test_fun) < 0.01000000001);
  REQUIRE(fun_set2.Run(-0.1, test_fun) > 0.00999999999);
}
