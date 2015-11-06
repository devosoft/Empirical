// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/FunctionSet.h"

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

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // TEST 1: Functions with VOID returns.
  emp::FunctionSet<void, int,int> fun_set;
  fun_set.Add(&fun1);
  fun_set.Add(&fun2);
  fun_set.Add(&fun3);
  fun_set.Add(&fun4);

  fun_set.Run(10, 2);
  
  if (verbose) {
    std::cout << "Ran list of void functions.  Results are:"
              << " " << global_var1
              << " " << global_var2
              << " " << global_var3
              << " " << global_var4
              << std::endl;
  }

  emp_assert(global_var1 == 12);
  emp_assert(global_var2 == 8);
  emp_assert(global_var3 == 20);
  emp_assert(global_var4 == 5);

  // Test 2: Functions with non-void returns.
  emp::FunctionSet<double, double> fun_set2;
  fun_set2.Add(&fun5);
  fun_set2.Add(&fun6);
  fun_set2.Add(&fun7);

  std::vector<double> out_vals;

  out_vals = fun_set2.Run(10.0);
  if (verbose) {
    for (double cur_val : out_vals) std::cout << cur_val << "  ";
    std::cout << std::endl;
  }
  emp_assert(out_vals[0] == 10.0);
  emp_assert(out_vals[1] == 100.0);
  emp_assert(out_vals[2] == 1000.0);

  out_vals = fun_set2.Run(-10.0);
  if (verbose) {
    for (double cur_val : out_vals) std::cout << cur_val << "  ";
    std::cout << std::endl;
  }
  emp_assert(out_vals[0] == -10.0);
  emp_assert(out_vals[1] == 100.0);
  emp_assert(out_vals[2] == -1000.0);

  out_vals = fun_set2.Run(0.1);
  if (verbose) {
    for (double cur_val : out_vals) std::cout << cur_val << "  ";
    std::cout << std::endl;
  }
  emp_assert(out_vals[0] == 0.1);
  emp_assert(out_vals[1] < 0.01000001);
  emp_assert(out_vals[1] > 0.00999999);
  emp_assert(out_vals[2] < 0.00100001);
  emp_assert(out_vals[2] > 0.00099999);

  out_vals = fun_set2.Run(-0.1);
  if (verbose) {
    for (double cur_val : out_vals) std::cout << cur_val << "  ";
    std::cout << std::endl;
  }
  emp_assert(out_vals[0] == -0.1);
  emp_assert(out_vals[1] < 0.01000001);
  emp_assert(out_vals[1] > 0.00999999);
  emp_assert(out_vals[2] > -0.00100001);
  emp_assert(out_vals[2] < -0.00099999);


  // TEST 3: Running functions and auto-processing return values.
  if (verbose) { 
    std::cout << std::endl;
    std::cout << "Find Maximum" << std::endl;
    std::cout << fun_set2.FindMax(10.0) << std::endl;
    std::cout << fun_set2.FindMax(-10.0) << std::endl;
    std::cout << fun_set2.FindMax(0.1) << std::endl;
    std::cout << fun_set2.FindMax(-0.1) << std::endl;
  }
  emp_assert(fun_set2.FindMax(10.0) == 1000.0);
  emp_assert(fun_set2.FindMax(-10.0) == 100.0);
  emp_assert(fun_set2.FindMax(0.1) == 0.1);
  emp_assert(fun_set2.FindMax(-0.1) < 0.0100000001);
  emp_assert(fun_set2.FindMax(-0.1) > 0.0099999999);

  if (verbose) { 
    std::cout << std::endl;
    std::cout << "Find Minimum" << std::endl;
    std::cout << fun_set2.FindMin(10.0) << std::endl;
    std::cout << fun_set2.FindMin(-10.0) << std::endl;
    std::cout << fun_set2.FindMin(0.1) << std::endl;
    std::cout << fun_set2.FindMin(-0.1) << std::endl;
  }
  emp_assert(fun_set2.FindMin(10.0) == 10.0);
  emp_assert(fun_set2.FindMin(-10.0) == -1000.0);
  emp_assert(fun_set2.FindMin(0.1) < 0.0010000001);
  emp_assert(fun_set2.FindMin(0.1) > 0.0009999999);
  emp_assert(fun_set2.FindMin(-0.1) == -0.1);
  
  if (verbose) { 
    std::cout << std::endl;
    std::cout << "Find Sum" << std::endl;
    std::cout << fun_set2.FindSum(10.0) << std::endl;
    std::cout << fun_set2.FindSum(-10.0) << std::endl;
    std::cout << fun_set2.FindSum(0.1) << std::endl;
    std::cout << fun_set2.FindSum(-0.1) << std::endl;
  }
  emp_assert(fun_set2.FindSum(10.0) == 1110.0);
  emp_assert(fun_set2.FindSum(-10.0) == -910.0);
  emp_assert(fun_set2.FindSum(0.1) < 0.1110000001);
  emp_assert(fun_set2.FindSum(0.1) > 0.1109999999);
  emp_assert(fun_set2.FindSum(-0.1) == -0.091);


  // TEST 4: Manually-entered aggregation function.
  std::function<double(double,double)> test_fun =
    [](double i1, double i2){ return std::max(i1,i2); };
  if (verbose) {
    std::cout << std::endl;
    std::cout << "Find Maximum (manual)" << std::endl;
    std::cout << fun_set2.Run(10.0, test_fun) << std::endl;
    std::cout << fun_set2.Run(-10.0, test_fun) << std::endl;
    std::cout << fun_set2.Run(0.1, test_fun) << std::endl;
    std::cout << fun_set2.Run(-0.1, test_fun) << std::endl;
  }
  emp_assert(fun_set2.Run(10.0, test_fun) == 1000.0);
  emp_assert(fun_set2.Run(-10.0, test_fun) == 100.0);
  emp_assert(fun_set2.Run(0.1, test_fun) == 0.1);
  emp_assert(fun_set2.Run(-0.1, test_fun) < 0.01000000001);
  emp_assert(fun_set2.Run(-0.1, test_fun) > 0.00999999999);

  return 0;
}
