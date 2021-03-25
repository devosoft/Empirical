//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/meta/reflection.hpp"
#include "emp/base/macros.hpp"


#define SHOW_MACRO(...) #__VA_ARGS__ " = " EMP_STRINGIFY( __VA_ARGS__ )
#define PRINT_MACRO(...) std::cout << #__VA_ARGS__ " = " EMP_STRINGIFY( __VA_ARGS__ ) << '\n'

#define TEST_MARK(X) x ## X ## x

void testing(int a) { std::cout << a << '\n'; }
void testing(int a, int b) { std::cout << a+b << '\n'; }
void testing(int a, int b, int c) { std::cout << a+b+c << '\n'; }
void testing(int a, int b, int c, int d) { std::cout << a+b+c+d << '\n'; }
void testing(int a, int b, int c, int d, int e) { std::cout << a+b+c+d+e << '\n'; }

#define TEST_SETS (1,2), (3,4,5,6), (7,8,9)



int main()
{
  std::cout << "Testing." << '\n';

  std::cout << SHOW_MACRO( TEST_MARK(0) ) << '\n';
  PRINT_MACRO( TEST_MARK(1) );

  PRINT_MACRO( EMP_WRAP_ARGS( TEST_MARK, 1, 2, 3, 4 ) );
  PRINT_MACRO( EMP_WRAP_EACH( TEST_MARK, 1, 2, 3, 4 ) );
  PRINT_MACRO( EMP_CALL_BY_PACKS(EMP_WRAP_EACH_, TEST_MARK, 1, 2, 3, 4 ) );

  PRINT_MACRO( EMP_TYPES_TO_VALS(int,double,std::string) );
  PRINT_MACRO( EMP_TYPES_TO_VALS( double , double) );

  std::cout << EMP_STRINGIFY( EMP_PERMUTE(XYZ) ) << '\n' << '\n';
  std::cout << EMP_STRINGIFY( EMP_PERMUTE(A,B) ) << '\n' << '\n';
  std::cout << EMP_STRINGIFY( EMP_PERMUTE(1,2,3) ) << '\n' << '\n';

  // @CAO PERMUTE needs more work!
  // std::cout << EMP_STRINGIFY( EMP_PERMUTE(a,b,c,d) ) << '\n' << '\n';
  // std::cout << EMP_STRINGIFY( EMP_PERMUTE(v,w,x,y,z) ) << '\n' << '\n';

  // EMP_WRAP_EACH_1ARG(;testing, 1, 10, 20, 30);
  //
  // EMP_WRAP_EACH(;testing, TEST_SETS);
  // EMP_PACKS_POP_ALL((1,2), (3,4,5,6), (7,8,9));
  // EMP_PACKS_PUSH_ALL(XXX, (1,2), (3,4,5,6), (7,8,9));

  std::cout << EMP_STRINGIFY(
			     EMP_REORDER_ARGS_IMPL( (int test), ((int, a), (bool, b), (std::string, c)) )
	       ) << '\n';


  // Explore EMP_IF!
  std::cout << "EMP_IF(0, \"true\", \"false\") = " << EMP_IF(0, "true", "false") << '\n';
  std::cout << "EMP_IF(1, \"true\", \"false\") = " << EMP_IF(1, "true", "false") << '\n';
  std::cout << "EMP_IF(2, \"true\", \"false\") = " << EMP_IF(2, "true", "false") << '\n';
  std::cout << "EMP_IF(three, \"true\", \"false\") = " << EMP_IF(three, "true", "false") << '\n';
  std::cout << "EMP_IF( EMP_EQU(4,4), \"true\", \"false\") = " << EMP_IF(EMP_EQU(4,4), "true", "false") << '\n';
  std::cout << "EMP_IF( EMP_EQU(5,6), \"true\", \"false\") = " << EMP_IF(EMP_EQU(5,6), "true", "false") << '\n';
}
