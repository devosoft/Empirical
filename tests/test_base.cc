//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Tests for files in the base/ folder.

#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "../third-party/Catch/single_include/catch.hpp"

#include <sstream>
#include <string>

#include "base/array.h"
#include "base/assert.h"
#include "base/errors.h"
#include "base/macro_math.h"
#include "base/macros.h"
#include "base/Ptr.h"
#include "base/vector.h"


// "Macros testing macros...Oh dear..."
#undef EMP_TEST_MACRO
#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                             \
  do {                                                                  \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));           \
    REQUIRE(result == EXP_RESULT);                                      \
  } while (false)


TEST_CASE("Test array", "[base]")
{
  constexpr int A_SIZE = 50;
  emp::array<int, A_SIZE> test_array;

  for (size_t i = 0; i < A_SIZE; i++) {
    test_array[i] = (int) (i * i);
  }

  int sum = 0;
  for (size_t i = 0; i < A_SIZE; i++) {
    sum += test_array[i];
  }

  REQUIRE(sum == 40425);
}


TEST_CASE("Test macro_math", "[base]")
{

  // Test converting between binary, decimal, and sum formats.
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(9), "0, 0, 0, 0, 0, 0, 1, 0, 0, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(91), "0, 0, 0, 1, 0, 1, 1, 0, 1, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_BIN(999), "1, 1, 1, 1, 1, 0, 0, 1, 1, 1");

  EMP_TEST_MACRO( EMP_BIN_TO_DEC(0,0,0,0,0,0,1,0,1,1), "11");
  EMP_TEST_MACRO( EMP_BIN_TO_DEC(0,0,0,1,0,1,1,0,1,1), "91");
  EMP_TEST_MACRO( EMP_BIN_TO_DEC(1,0,1,0,1,0,1,0,1,0), "682");

  EMP_TEST_MACRO( EMP_BIN_TO_SUM(0,0,0,1,0,1,1,0,1,1), "0, 0, 0, 64, 0, 16, 8, 0, 2, 1");
  EMP_TEST_MACRO( EMP_DEC_TO_SUM(91), "0, 0, 0, 64, 0, 16, 8, 0, 2, 1");

  EMP_TEST_MACRO( EMP_BIN_TO_PACK(0,0,0,1,0,1,1,0,1,1), "(64, 16, 8, 2, 1)");
  EMP_TEST_MACRO( EMP_DEC_TO_PACK(91), "(64, 16, 8, 2, 1)");

  // Test Boolean logic
  EMP_TEST_MACRO( EMP_NOT(0), "1" );
  EMP_TEST_MACRO( EMP_NOT(EMP_NOT(0)), "0" );

  EMP_TEST_MACRO( EMP_BIT_EQU(0,0), "1" );
  EMP_TEST_MACRO( EMP_BIT_EQU(0,1), "0" );
  EMP_TEST_MACRO( EMP_BIT_EQU(1,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_EQU(1,1), "1" );

  EMP_TEST_MACRO( EMP_BIT_LESS(0,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_LESS(0,1), "1" );
  EMP_TEST_MACRO( EMP_BIT_LESS(1,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_LESS(1,1), "0" );

  EMP_TEST_MACRO( EMP_BIT_GTR(0,0), "0" );
  EMP_TEST_MACRO( EMP_BIT_GTR(0,1), "0" );
  EMP_TEST_MACRO( EMP_BIT_GTR(1,0), "1" );
  EMP_TEST_MACRO( EMP_BIT_GTR(1,1), "0" );

  // Test conditionals.
  EMP_TEST_MACRO( EMP_IF_impl_0(abc), "~, abc" );
  EMP_TEST_MACRO( EMP_IF_impl_1(abc), "EMP_IF_impl_1(abc)" );
  EMP_TEST_MACRO( EMP_IF(0, A, B), "B" );
  EMP_TEST_MACRO( EMP_IF(1, A, B), "A" );

  // Test comparisons
  EMP_TEST_MACRO( EMP_COMPARE(10,20), "B" );
  EMP_TEST_MACRO( EMP_COMPARE(1023,1022), "A" );
  EMP_TEST_MACRO( EMP_COMPARE(1000,999), "A" );
  EMP_TEST_MACRO( EMP_COMPARE(678,678), "X" );

  EMP_TEST_MACRO( EMP_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_EQU(2,5), "0" );
  EMP_TEST_MACRO( EMP_EQU(5,8), "0" );
  EMP_TEST_MACRO( EMP_EQU(8,5), "0" );
  EMP_TEST_MACRO( EMP_EQU(5,2), "0" );

  EMP_TEST_MACRO( EMP_LESS(5,5), "0" );
  EMP_TEST_MACRO( EMP_LESS(2,5), "1" );
  EMP_TEST_MACRO( EMP_LESS(5,8), "1" );
  EMP_TEST_MACRO( EMP_LESS(8,5), "0" );
  EMP_TEST_MACRO( EMP_LESS(5,2), "0" );

  EMP_TEST_MACRO( EMP_LESS_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(2,5), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(5,8), "1" );
  EMP_TEST_MACRO( EMP_LESS_EQU(8,5), "0" );
  EMP_TEST_MACRO( EMP_LESS_EQU(5,2), "0" );

  EMP_TEST_MACRO( EMP_GTR(5,5), "0" );
  EMP_TEST_MACRO( EMP_GTR(2,5), "0" );
  EMP_TEST_MACRO( EMP_GTR(5,8), "0" );
  EMP_TEST_MACRO( EMP_GTR(8,5), "1" );
  EMP_TEST_MACRO( EMP_GTR(5,2), "1" );

  EMP_TEST_MACRO( EMP_GTR_EQU(5,5), "1" );
  EMP_TEST_MACRO( EMP_GTR_EQU(2,5), "0" );
  EMP_TEST_MACRO( EMP_GTR_EQU(5,8), "0" );
  EMP_TEST_MACRO( EMP_GTR_EQU(8,5), "1" );
  EMP_TEST_MACRO( EMP_GTR_EQU(5,2), "1" );

  EMP_TEST_MACRO( EMP_NEQU(5,5), "0" );
  EMP_TEST_MACRO( EMP_NEQU(2,5), "1" );
  EMP_TEST_MACRO( EMP_NEQU(5,8), "1" );
  EMP_TEST_MACRO( EMP_NEQU(8,5), "1" );
  EMP_TEST_MACRO( EMP_NEQU(5,2), "1" );



  // Test other helper math functions.
  EMP_TEST_MACRO( EMP_MATH_VAL_TIMES_0(222), "0" );
  EMP_TEST_MACRO( EMP_MATH_VAL_TIMES_1(222), "222" );

  EMP_TEST_MACRO( EMP_MATH_BIN_TIMES_0(0,0,1,0,1,0,1,0,1,0), "0, 0, 0, 0, 0, 0, 0, 0, 0, 0" );
  EMP_TEST_MACRO( EMP_MATH_BIN_TIMES_1(0,0,1,0,1,0,1,0,1,0), "0, 0, 1, 0, 1, 0, 1, 0, 1, 0" );

  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(1, 1), "2");
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(1, N), "0");
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(0, N), "N");

  EMP_TEST_MACRO( EMP_MATH_GET_CARRY(2), "1");
  EMP_TEST_MACRO( EMP_MATH_CLEAR_CARRY(2), "0");

  // Now in combination...
  EMP_TEST_MACRO( EMP_MATH_COUNT_BITS(EMP_MATH_CLEAR_CARRY(1), EMP_MATH_GET_CARRY(2)), "2" );

  // Basic Addition...
  EMP_TEST_MACRO( EMP_ADD(1, 2), "3");
  EMP_TEST_MACRO( EMP_ADD(5, 5), "10");
  EMP_TEST_MACRO( EMP_ADD(7, 7), "14");
  EMP_TEST_MACRO( EMP_ADD(111, 112), "223");
  EMP_TEST_MACRO( EMP_ADD(127, 1), "128");
  EMP_TEST_MACRO( EMP_ADD(123, 789), "912");
  EMP_TEST_MACRO( EMP_ADD(1023, 1), "0");      // Overflow

  EMP_TEST_MACRO( EMP_ADD_10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10), "55" );

  // Basic Subtraction...
  EMP_TEST_MACRO( EMP_SUB(10, 7), "3");
  EMP_TEST_MACRO( EMP_SUB(128, 1), "127");
  EMP_TEST_MACRO( EMP_SUB(250, 250), "0");
  EMP_TEST_MACRO( EMP_SUB(250, 100), "150");
  EMP_TEST_MACRO( EMP_SUB(91, 66), "25");
  EMP_TEST_MACRO( EMP_SUB(99, 100), "1023");   // Underflow

  // Combination of add and sub
  EMP_TEST_MACRO( EMP_ADD( EMP_SUB(250, 100), EMP_SUB(91, 66)), "175");

  // Shifting
  EMP_TEST_MACRO( EMP_SHIFTL(17), "34");
  EMP_TEST_MACRO( EMP_SHIFTL(111), "222");
  EMP_TEST_MACRO( EMP_SHIFTL(444), "888");
  EMP_TEST_MACRO( EMP_SHIFTL(1023), "1022");   // Overflow...

  EMP_TEST_MACRO( EMP_SHIFTR(100), "50");
  EMP_TEST_MACRO( EMP_SHIFTR(151), "75");

  EMP_TEST_MACRO( EMP_SHIFTL_X(0, 700), "700");
  EMP_TEST_MACRO( EMP_SHIFTL_X(5, 17),  "544");
  EMP_TEST_MACRO( EMP_SHIFTL_X(1, 111), "222");
  EMP_TEST_MACRO( EMP_SHIFTR_X(1, 100), "50");
  EMP_TEST_MACRO( EMP_SHIFTR_X(3, 151), "18");

  // Inc, dec, half...
  EMP_TEST_MACRO( EMP_INC(20), "21");
  EMP_TEST_MACRO( EMP_INC(55), "56");
  EMP_TEST_MACRO( EMP_INC(63), "64");
  EMP_TEST_MACRO( EMP_INC(801), "802");

  EMP_TEST_MACRO( EMP_DEC(20), "19");
  EMP_TEST_MACRO( EMP_DEC(55), "54");
  EMP_TEST_MACRO( EMP_DEC(63), "62");
  EMP_TEST_MACRO( EMP_DEC(900), "899");

  EMP_TEST_MACRO( EMP_HALF(17), "8");
  EMP_TEST_MACRO( EMP_HALF(18), "9");
  EMP_TEST_MACRO( EMP_HALF(60), "30");
  EMP_TEST_MACRO( EMP_HALF(1001), "500");

  // Multiply!
  EMP_TEST_MACRO( EMP_MULT(1, 1), "1");
  EMP_TEST_MACRO( EMP_MULT(200, 0), "0");
  EMP_TEST_MACRO( EMP_MULT(201, 1), "201");
  EMP_TEST_MACRO( EMP_MULT(10, 7), "70");
  EMP_TEST_MACRO( EMP_MULT(25, 9), "225");
  EMP_TEST_MACRO( EMP_MULT(65, 3), "195");
  EMP_TEST_MACRO( EMP_MULT(65, 15), "975");

  // Bit Manipulation!
  EMP_TEST_MACRO( EMP_COUNT_ONES(0), "0");
  EMP_TEST_MACRO( EMP_COUNT_ONES(509), "8");
  EMP_TEST_MACRO( EMP_COUNT_ONES(1023), "10");

  EMP_TEST_MACRO( EMP_LOG2(0), "0" );
  EMP_TEST_MACRO( EMP_LOG2(1), "1" );
  EMP_TEST_MACRO( EMP_LOG2(3), "2" );
  EMP_TEST_MACRO( EMP_LOG2(5), "3" );
  EMP_TEST_MACRO( EMP_LOG2(10), "4" );
  EMP_TEST_MACRO( EMP_LOG2(20), "5" );
  EMP_TEST_MACRO( EMP_LOG2(40), "6" );
  EMP_TEST_MACRO( EMP_LOG2(75), "7" );
  EMP_TEST_MACRO( EMP_LOG2(150), "8" );
  EMP_TEST_MACRO( EMP_LOG2(300), "9" );
  EMP_TEST_MACRO( EMP_LOG2(600), "10" );

  // Division!
  EMP_TEST_MACRO( EMP_DIV_start(2), "8" );
  EMP_TEST_MACRO( EMP_DIV(8, 2), "4" );
  EMP_TEST_MACRO( EMP_DIV(100, 5), "20" );
  EMP_TEST_MACRO( EMP_DIV(1000, 17), "58" );

  // Modulus!
  EMP_TEST_MACRO( EMP_MOD(10, 3), "1" );
  EMP_TEST_MACRO( EMP_MOD(127, 10), "7" );
  EMP_TEST_MACRO( EMP_MOD(127, 1000), "127" );
  EMP_TEST_MACRO( EMP_MOD(102, 3), "0" );
}



TEST_CASE("Test macros", "[base]")
{
  EMP_TEST_MACRO( EMP_POP_ARGS_32(1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "3,4,5,6,7,8,9,0");
  EMP_TEST_MACRO( EMP_POP_ARGS(32, 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "3,4,5,6,7,8,9,0");
  EMP_TEST_MACRO( EMP_POP_ARGS(39, 1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0), "0");

  // Test getting a specific argument.
  EMP_TEST_MACRO( EMP_POP_ARGS( EMP_DEC(5), 11,12,13,14,15,16,17 ), "15,16,17");
  EMP_TEST_MACRO( EMP_GET_ARG(5, 11,12,13,14,15,16,17), "15");

  // Test counting number of arguments.
  EMP_TEST_MACRO( EMP_COUNT_ARGS(a, b, c), "3" );
  EMP_TEST_MACRO( EMP_COUNT_ARGS(x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x), "600" );

  // Make sure EMP_STRINGIFY can process macros before running
  EMP_TEST_MACRO( EMP_STRINGIFY(EMP_MERGE(ab, c, de, f)), "\"abcdef\"");
  EMP_TEST_MACRO( EMP_STRINGIFY("abcdef"), "\"\\\"abcdef\\\"\"" );

  // Test new range macros.
  EMP_TEST_MACRO( EMP_RANGE(901, 905), "901 , 902 , 903 , 904 , 905" );

  // Test PACK manipulation
  EMP_TEST_MACRO( EMP_PACK_ARGS(a,b,c), "(a,b,c)");
  EMP_TEST_MACRO( EMP_UNPACK_ARGS((a,b,c)), "a,b,c");
  EMP_TEST_MACRO( EMP_PACK_POP((a,b,c)), "(b,c)");
  EMP_TEST_MACRO( EMP_PACK_TOP((a,b,c)), "a");
  EMP_TEST_MACRO( EMP_PACK_PUSH(x, (a,b,c)), "(x,a,b,c)");
  EMP_TEST_MACRO( EMP_PACK_SIZE((a,b,c)), "3");

  // BAD TEST: EMP_TEST_MACRO( EMP_ARGS_TO_PACKS_1(4, a,b,c,d,e,f,g), "(a , b , c , d)" );


  EMP_TEST_MACRO( EMP_CALL_BY_PACKS(TST_, (Fixed), a,b,c,d,e,f,g,h,i,j,k,l,m), "TST_8((Fixed), a,b,c,d,e,f,g,h,i,j,k,l,m, ~) TST_4((Fixed), i,j,k,l,m, ~) TST_1((Fixed), m, ~)" );

  // Make sure we can wrap each argument in a macro.
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o] [p]" );
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o]" );
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m]" );


  // Test replacement of commas
  EMP_TEST_MACRO( EMP_REPLACE_COMMAS(~, x,x,x,x,x,x,x), "x ~ x ~ x ~ x ~ x ~ x ~ x" );
  EMP_TEST_MACRO( EMP_REPLACE_COMMAS(%, x,x,x,x,x,x,x,x), "x % x % x % x % x % x % x % x" );


  // Simple argument manipulation
  EMP_TEST_MACRO( EMP_ROTATE_ARGS(a, b, c), "b, c, a" );

  // Test trimming argument lists.
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b,c,d,e,f,g,h,i ), "a, c, d, f, g, i" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x), 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50 ), "1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49" );


  // Test more complex layouts...
  EMP_TEST_MACRO( EMP_LAYOUT(EMP_DECORATE, +, a, b, c, d, e, f, g, h), "[a] + [b] + [c] + [d] + [e] + [f] + [g] + [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARGS(EMP_DECORATE, a, b, c, d, e, f, g, h), "[a] , [b] , [c] , [d] , [e] , [f] , [g] , [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARG_PAIRS(EMP_DECORATE_PAIR, A, a, B, b, C, c, D, d, E, e, F, f), "[A-a], [B-b], [C-c], [D-d], [E-e], [F-f]" );


  // Rest controlling argument number.
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(15, x), "x, x, x, x, x, x, x, x, x, x, x, x, x, x, x" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(5, x,y,z), "x,y,z, x,y,z, x,y,z, x,y,z, x,y,z" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(63, 123), "123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123" );

  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(26, x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x),
                  "x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x , x" );

  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(5, a, b, c, d, e, f, g, h, i, j), "a , b , c , d , e" );
  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(4, a, b, c, d, e, f, g, h, i, j), "a , b , c , d" );
  EMP_TEST_MACRO( EMP_CROP_ARGS_TO(4, a, b, c, d), "a , b , c , d" );

  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(3, x, a, b, c, d), "a , b , c" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(4, x, a, b, c, d), "a , b , c , d" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(7, x, a, b, c, d), "a , b , c , d , x , x , x" );

  // Test collect only-odd or only-even arguments.
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "1, 3, 5, 7, 9, 11");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "2, 4, 6, 8, 10, 12");
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "1, 3, 5, 7, 9, 11, 13");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "2, 4, 6, 8, 10, 12");


  EMP_TEST_MACRO( EMP_REVERSE_ARGS(a,b,c, d), "d, c, b, a" );

  EMP_TEST_MACRO( EMP_DECLARE_VARS(int, char, bool, std::string),
                  "int arg1, char arg2, bool arg3, std::string arg4" );
  EMP_TEST_MACRO( EMP_NUMS_TO_VARS(4), "arg1 , arg2 , arg3 , arg4" );


  // Test EMP_STRINGIFY_EACH
  std::array<std::string, 2> test = {{ EMP_STRINGIFY_EACH(some, words) }};
  std::array<std::string, 9> test9 =
    {{ EMP_STRINGIFY_EACH(one, two, three, four, five, six, seven, eight, nine) }};

  REQUIRE(test.size() == 2);
  REQUIRE(test[0] == "some");
  REQUIRE(test[1] == "words");
  REQUIRE(test9.size() == 9);
  REQUIRE(test9[4] == "five");
  REQUIRE(test9[7] == "eight");

  EMP_TEST_MACRO( EMP_STRINGIFY_EACH(some, words), "\"some\" , \"words\"" );
}


TEST_CASE("Test errors", "[tools]")
{
  emp::TriggerExcept("test_fail", "The test failed.  *sob*");
  emp::TriggerExcept("test_fail2", "The second test failed too.  But it's not quite as aweful.", false);
  emp::TriggerExcept("test_fail2", "The third test is just test 2 again, but worse", true);

  REQUIRE( emp::CountExcepts() == 3 );
  auto except = emp::PopExcept("test_fail2");
  REQUIRE( emp::CountExcepts() == 2 );
  REQUIRE( except.desc == "The second test failed too.  But it's not quite as aweful." );
  REQUIRE( emp::HasExcept("test_fail2") == true );
  REQUIRE( emp::HasExcept("test_fail3") == false );
  emp::ClearExcepts();
  REQUIRE( emp::CountExcepts() == 0 );
}


TEST_CASE("Test Ptr", "[base]")
{
  // Test default constructor.
  emp::Ptr<int> ptr1;
  ptr1.New();
  *ptr1 = 5;
  REQUIRE(*ptr1 == 5);
  ptr1.Delete();

  // Test pointer constructor
  int * temp_int = new int;
  emp::Ptr<int> ptr2(temp_int, true);
  *ptr2 = 10;
  REQUIRE(*ptr2 == 10);
  ptr2.Delete();

  // Test non-pointer object constructor
  int base_val = 15;
  emp::Ptr<int> ptr3(&base_val);
  REQUIRE(*ptr3 == 15);
  base_val = 20;                 // Make sure pointed to value changes with original variable.
  REQUIRE(*ptr3 == 20);

  // Test copy-constructor.
  emp::Ptr<int> ptr4(ptr3);
  REQUIRE(*ptr4 == 20);
  *ptr4 = 25;                    // Change this new pointer...
  REQUIRE(*ptr4 == 25);       // ...make sure it actually changed.
  REQUIRE(*ptr3 == 25);       // ...make sure the other pointer reflects the change.
  REQUIRE(base_val == 25);    // ...make sure the original variable changed.

  // -- Test count tracking on emp::Ptr --
  // A bit of an odd set of test... we need to create and destory pointers to make sure
  // that all of the counts are correct, so we're going to use arrays of pointers to them.

  emp::vector<emp::Ptr<char> *> ptr_set(10);
  ptr_set[0] = new emp::Ptr<char>;
  ptr_set[0]->New(42);
  for (size_t i = 1; i < 10; i++) ptr_set[i] = new emp::Ptr<char>(*(ptr_set[0]));

  // Do we have a proper count of 10?
  #ifdef EMP_TRACK_MEM
  REQUIRE(ptr_set[0]->DebugGetCount() == 10);
  ptr_set[1]->New(91);
  REQUIRE(ptr_set[0]->DebugGetCount() == 9);
  *(ptr_set[2]) = *(ptr_set[1]);
  REQUIRE(ptr_set[0]->DebugGetCount() == 8);
  REQUIRE(ptr_set[1]->DebugGetCount() == 2);

  ptr_set[3]->Delete();
  ptr_set[1]->Delete();
  #endif 

  // Make sure that we are properly handling temporary pointers moved to uninitialized pointes.
  // (Previously breaking, now fixed.)
  int a = 9;
  emp::Ptr<int> ptr_a;
  ptr_a = emp::ToPtr(&a);
  int a_val = *(ptr_a);
  REQUIRE(a_val == 9);

  // Test casting to unsigned char
  emp::Ptr<uint32_t> ptr5;
  ptr5.New();
  *ptr5 = 1+1024;

  REQUIRE(*ptr5.Cast<unsigned char>() == 1);
  ptr5.Delete();

  // Test casting to const unsigned char
  emp::Ptr<uint32_t> ptr6;
  ptr6.New();
  *ptr6 = 6+1024;

  REQUIRE(*ptr6.Cast<const unsigned char>() == 6);
  ptr6.Delete();

  // Test casting to const unsigned char
  emp::Ptr<uint32_t> ptr7;
  ptr7.New();
  *ptr7 = 6+1024;
  const emp::Ptr<const unsigned char> ptr8 = ptr7.Cast<const unsigned char>();
  REQUIRE(*ptr8 == 6);
  ptr7.Delete();

  // std::cout << ptr_set[0]->DebugGetCount() << std::endl;

//   // @CAO Make sure we don't delete below 0
//   // @CAO Make sure we don't delete below 1 if we own it
//   // @CAO Make sure we only delete if you own it
//   // @CAO Make sure not to delete twice!
//   // @CAO Make sure we don't add (as owner) a pointer we already own
//
//   // -- Do some direct tests on pointer trackers --
//
//   int * real_ptr1 = new int(1);  // Count of 2 in tracker
//   int * real_ptr2 = new int(2);  // Deleted in tracker
//   int * real_ptr3 = new int(3);  // Unknown to tracker
//   int * real_ptr4 = new int(4);  // Passively known to tracker (marked non-owner)
//   auto & tracker = emp::PtrTracker::Get();
//
//   tracker.New(real_ptr1);
//   tracker.Inc(real_ptr1);
//   tracker.Inc(real_ptr1);
//   tracker.Dec(real_ptr1);
//
//   tracker.New(real_ptr2);
//   tracker.MarkDeleted(real_ptr2);
//
//   tracker.Old(real_ptr4);
//
//   REQUIRE(tracker.HasPtr(real_ptr1) == true);
//   REQUIRE(tracker.HasPtr(real_ptr2) == true);
// //  REQUIRE(tracker.HasPtr(real_ptr3) == false);  // Technically may be previous pointer re-used!
//   REQUIRE(tracker.HasPtr(real_ptr4) == true);
//
//   REQUIRE(tracker.IsActive(real_ptr1) == true);
//   REQUIRE(tracker.IsActive(real_ptr2) == false);
// //  REQUIRE(tracker.IsActive(real_ptr3) == false);
//   REQUIRE(tracker.IsActive(real_ptr4) == true);
//
//   REQUIRE(tracker.IsOwner(real_ptr1) == true);
//   REQUIRE(tracker.IsOwner(real_ptr2) == true);
// //  REQUIRE(tracker.IsOwner(real_ptr3) == false);
//   REQUIRE(tracker.IsOwner(real_ptr4) == false);
//
//   REQUIRE(tracker.GetCount(real_ptr1) == 2);
//   REQUIRE(tracker.GetCount(real_ptr2) == 1);
// //  REQUIRE(tracker.GetCount(real_ptr3) == 0);
//   REQUIRE(tracker.GetCount(real_ptr4) == 1);
}


TEST_CASE("Test vector", "[base]")
{
  emp::vector<int> v(20);

  for (size_t i = 0; i < 20; i++) {
    v[i] = (int) (i * i);
  }

  int total = 0;
  for (int i : v) total += i;

  REQUIRE(total == 2470);

  // Examine vector<bool> specialization.
  emp::vector<bool> vb(1000,false);
  for (size_t i = 0; i < vb.size(); i++) {
    if (i%3==0 || i%5 == 0) vb[i] = true;
  }
  size_t count = 0;
  const auto vb2 = vb;
  for (size_t i = 0; i < vb.size(); i++) {
    if (vb2[i]) count++;
  }

  REQUIRE(count == 467);
}

TEST_CASE("Replicate ptr bug", "[ptr]") {
  struct testA {
    int a = 9;
    emp::Ptr<int> GetA() {return emp::ToPtr(&a);}
  };

  struct testB {
    std::function<emp::Ptr<int>(void)> b_fun;
    emp::Ptr<int> b;
    
    void SetBFun(std::function<emp::Ptr<int>(void)> fun) {
      b_fun = fun;
    }

    void RunBFun() {
      b = b_fun();
    }

  };

  testA ta;
  testB tb;
  
  std::function<emp::Ptr<int>(void)> return_a = [&ta](){return ta.GetA();};
  tb.SetBFun(return_a);
  tb.RunBFun();
  REQUIRE(*(tb.b) == 9);

}
