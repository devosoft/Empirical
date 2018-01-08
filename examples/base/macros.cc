//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "meta/reflection.h"
#include "base/macros.h"

void testing(int a) { std::cout << a << std::endl; }
void testing(int a, int b) { std::cout << a+b << std::endl; }
void testing(int a, int b, int c) { std::cout << a+b+c << std::endl; }
void testing(int a, int b, int c, int d) { std::cout << a+b+c+d << std::endl; }
void testing(int a, int b, int c, int d, int e) { std::cout << a+b+c+d+e << std::endl; }

#define TEST_SETS (1,2), (3,4,5,6), (7,8,9)



int main()
{
  std::cout << "Testing." << std::endl;

  std::cout << EMP_STRINGIFY( EMP_PERMUTE(XYZ) ) << std::endl << std::endl;
  std::cout << EMP_STRINGIFY( EMP_PERMUTE(A,B) ) << std::endl << std::endl;
  std::cout << EMP_STRINGIFY( EMP_PERMUTE(1,2,3) ) << std::endl << std::endl;

  // @CAO PERMUTE needs more work!
  // std::cout << EMP_STRINGIFY( EMP_PERMUTE(a,b,c,d) ) << std::endl << std::endl;
  // std::cout << EMP_STRINGIFY( EMP_PERMUTE(v,w,x,y,z) ) << std::endl << std::endl;

  // EMP_WRAP_EACH_1ARG(;testing, 1, 10, 20, 30);
  //
  // EMP_WRAP_EACH(;testing, TEST_SETS);
  // EMP_PACKS_POP_ALL((1,2), (3,4,5,6), (7,8,9));
  // EMP_PACKS_PUSH_ALL(XXX, (1,2), (3,4,5,6), (7,8,9));

  std::cout << EMP_STRINGIFY(
			     EMP_REORDER_ARGS_IMPL( (int test), ((int, a), (bool, b), (std::string, c)) )
	       ) << std::endl;

}
