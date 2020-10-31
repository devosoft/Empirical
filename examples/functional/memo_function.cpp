//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::memo_function

#include <iostream>
#include "emp/functional/memo_function.hpp"
#include "emp/math/Random.hpp"

double F(int N) {
  double PI = 3.14159;
  double val = 1.0;
  while (N-- > 0) { val *= PI; if (val > 1000.0) val /= 1000.0; }
  return val;
}

double G(int N) {
  return 0.00005 * (double) N;
}

int Mult2(int in1, int in2) {
  std::cout << "!";
  return in1 * in2;
}

int Compute() {
  int value = 0;
  for (int i = 0; i < 1000000000; i++) {
    value += 111111;
    value *= 2;
    value %= 837645781;
  }
  return value;
}

int main()
{
  std::cout << "Testing.  " << std::endl;

  emp::memo_function<double(int)> test_fun(F);

  for (int i = 0; i < 200; i++) {
    std::cout << i%100 << ":" << test_fun(i%100+10000000) << " ";
    if (i%8 == 7) std::cout << std::endl;
  }

  // Change the test function; make sure we get a new set of results!
  // test_fun = std::function<double(int)>(G);
  test_fun = G;

  std::cout << std::endl; // Skip a line...
  for (int i = 0; i < 200; i++) {
    std::cout << i%100 << ":" << test_fun(i%100+10000000) << " ";
    if (i%8 == 7) std::cout << std::endl;
  }

  // Build a recursive memo_function...
  test_fun = [&test_fun](int N) {
    if (N<=1) return (double) N;
    return test_fun(N-1) + test_fun(N-2);
  };

  std::cout << std::endl; // Skip a line...
  for (int i = 80; i < 90; i++) {
    std::cout << i << ":" << test_fun(i) << " ";
    if (i%8 == 7) std::cout << std::endl;
  }

  emp::memo_function<int(int,int)> test_fun2(Mult2);
  emp::Random random;

  std::cout << std::endl; // Skip a line...
  std::cout << std::endl << "Multi-argument functions!";
  for (int i = 0; i < 1000; i++) {
    if (i % 20 == 0) std::cout << std::endl;
    int x = random.GetInt(30);
    int y = random.GetInt(30);
    std::cout << test_fun2(x,y) << " ";
  }
  std::cout << std::endl;

  emp::memo_function<double()> no_arg_fun(Compute);

  std::cout << no_arg_fun() << "  Long..." << std::endl;
  std::cout << no_arg_fun() << "  Quick!" << std::endl;
  std::cout << no_arg_fun() << "  Quick!" << std::endl;
  no_arg_fun = Compute;
  std::cout << no_arg_fun() << "  Long..." << std::endl;
}
