#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/macros.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp_assert(EMP_GET_ARG_5(11,12,13,14,15,16,17) == 15);

  emp_assert( std::string(EMP_STRINGIFY(EMP_MERGE(ab, c, de, f))) == "abcdef" );
  emp_assert( std::string(EMP_STRINGIFY("abcdef")) == "\"abcdef\"" );

  emp_assert( EMP_COUNT_ARGS(a, b, c) == 3 );

  emp_assert( std::string(EMP_STRINGIFY( EMP_ASSEMBLE_MACRO( EMP_GET_ARG_, 2, x, y, z ))) == "y" );


  std::cout << EMP_STRINGIFY(EMP_MERGE(a, bc, d, e, f, g, h)) << std::endl;
  std::cout << EMP_STRINGIFY( EMP_ASSEMBLE_MACRO( EMP_GET_ARG_, 2, x, y, z ) ) << std::endl;
}

