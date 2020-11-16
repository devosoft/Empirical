//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2020.
//  Released under the MIT Software license; see doc/LICENSE

#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]
#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/macros.hpp"

#include <array>
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
