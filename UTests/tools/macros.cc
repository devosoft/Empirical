#include <iostream>
#include <array>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/macros.h"
#include "../../tools/unit_tests.h"

#define EMP_DECORATE(X) [X]
#define EMP_DECORATE_PAIR(X,Y) [X-Y]

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // Test getting a specific argument.
  EMP_TEST_MACRO( EMP_GET_ARG_5(11,12,13,14,15,16,17), "15");

  // Test counting number of arguments.
  EMP_TEST_MACRO( EMP_COUNT_ARGS(a, b, c), "3" );

  // Make sure EMP_STRINGIFY can process macros before running
  EMP_TEST_MACRO( EMP_STRINGIFY(EMP_MERGE(ab, c, de, f)), "\"abcdef\"");
  EMP_TEST_MACRO( EMP_STRINGIFY("abcdef"), "\"\\\"abcdef\\\"\"" );

  // Make sure we can assemble arbitrary macros
  EMP_TEST_MACRO( EMP_ASSEMBLE_MACRO_1ARG( EMP_GET_ARG_, 2, x, y, z ), "y" );

  // Simple argument manipulation
  EMP_TEST_MACRO( EMP_ROTATE_ARGS(a, b, c), "b, c, a" );

  // Test trimming argument lists.
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b ), "a" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x,i), a,b,c,d,e,f,g,h,i ), "a, c, d, f, g, i" );
  EMP_TEST_MACRO( EMP_SELECT_ARGS( (i,x), 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50 ), "1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49" );


  // Make sure we can wrap each argument in a macro.
  EMP_TEST_MACRO( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p), "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o] [p]" );
  EMP_TEST_MACRO( EMP_LAYOUT(EMP_DECORATE, +, a, b, c, d, e, f, g, h), "[a] + [b] + [c] + [d] + [e] + [f] + [g] + [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARGS(EMP_DECORATE, a, b, c, d, e, f, g, h), "[a], [b], [c], [d], [e], [f], [g], [h]" );
  EMP_TEST_MACRO( EMP_WRAP_ARG_PAIRS(EMP_DECORATE_PAIR, A, a, B, b, C, c, D, d, E, e, F, f), "[A-a], [B-b], [C-c], [D-d], [E-e], [F-f]" );


  // Rest controlling argument number.
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(15, x), "x, x, x, x, x, x, x, x, x, x, x, x, x, x, x" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(5, x,y,z), "x,y,z, x,y,z, x,y,z, x,y,z, x,y,z" );
  EMP_TEST_MACRO( EMP_DUPLICATE_ARGS(63, 123), "123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123, 123" );

  EMP_TEST_MACRO( EMP_CROP_ARGS(4, a, b, c, d, e, f, g, h, i, j), "a, b, c, d" );
  EMP_TEST_MACRO( EMP_CROP_ARGS(4, a, b, c, d), "a, b, c, d" );
  
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(3, x, a, b, c, d), "a, b, c" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(4, x, a, b, c, d), "a, b, c, d" );
  EMP_TEST_MACRO( EMP_FORCE_ARGS_TO(7, x, a, b, c, d), "a, b, c, d, x, x, x" );

  // Test collect only-odd or only-even arguments.
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "1, 3, 5, 7, 9, 11");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12), "2, 4, 6, 8, 10, 12");
  EMP_TEST_MACRO( EMP_GET_ODD_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "1, 3, 5, 7, 9, 11, 13");
  EMP_TEST_MACRO( EMP_GET_EVEN_ARGS(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13), "2, 4, 6, 8, 10, 12");


  EMP_TEST_MACRO( EMP_REVERSE_ARGS(a,b,c, d), "d, c, b, a" );
  EMP_TEST_MACRO( EMP_TYPES_TO_ARGS(int, char, bool, std::string),
                  "int arg1, char arg2, bool arg3, std::string arg4" );


  // Test EMP_STRINGIFY_EACH
  std::array<std::string, 2> test = {EMP_STRINGIFY_EACH(some, words)};
  std::array<std::string, 9> test9 = {EMP_STRINGIFY_EACH(one, two, three, four, five, six, seven, eight, nine)};

  emp_assert(test.size() == 2);
  emp_assert(test[0] == "some");
  emp_assert(test[1] == "words");
  emp_assert(test9.size() == 9);
  emp_assert(test9[4] == "five");
  emp_assert(test9[7] == "eight");

  EMP_TEST_MACRO( EMP_STRINGIFY_EACH(some, words), "\"some\", \"words\"" );

  if (verbose) std::cout << "All tests passed." << std::endl;
}

