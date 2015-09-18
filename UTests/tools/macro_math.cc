#include <iostream>
#include <array>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/macro_math.h"
#include "../../tools/unit_tests.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // Test converting between binary and decimal.
  EMP_TEST_MACRO( EMP_VAL_TO_BIN(9), "1, 0, 0, 1");
  EMP_TEST_MACRO( EMP_BIN_TO_VAL(1,0,1,1), "11");

  // Test simple math.
  EMP_TEST_MACRO( EMP_COUNT_BITS(1, 1), "2");
  EMP_TEST_MACRO( EMP_MATH_GET_CARRY(2), "1");
  EMP_TEST_MACRO( EMP_MATH_CLEAR_CARRY(2), "0");
  
  // Now in combination...
  EMP_TEST_MACRO( EMP_COUNT_BITS(EMP_MATH_CLEAR_CARRY(1), EMP_MATH_GET_CARRY(2)), "2" );

  EMP_TEST_MACRO( EMP_ADD(1, 2), "3");
  EMP_TEST_MACRO( EMP_ADD(5, 5), "10");
  EMP_TEST_MACRO( EMP_ADD(7, 7), "14");
  EMP_TEST_MACRO( EMP_ADD(7, 1), "8");
  EMP_TEST_MACRO( EMP_ADD(15, 1), "0");

  if (verbose) std::cout << "All tests passed." << std::endl;
}
