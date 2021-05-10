#define CATCH_CONFIG_MAIN

//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2018.
//  Released under the MIT Software license; see doc/LICENSE

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/macro_math.hpp"
#include "emp/base/macros.hpp"

#include <sstream>
#include <iostream>
#include <string>

// "Macros testing macros...Oh dear..."
#undef EMP_TEST_MACRO
#define EMP_TEST_MACRO( MACRO, EXP_RESULT )                             \
  do {                                                                  \
    std::string result = std::string(EMP_STRINGIFY( MACRO ));           \
    REQUIRE(result == EXP_RESULT);                                      \
  } while (false)

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
