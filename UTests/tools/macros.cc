#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/macros.h"

#define EMP_DECORATE(X) [X]

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // Test getting a specific argument.
  emp_assert(EMP_GET_ARG_5(11,12,13,14,15,16,17) == 15);

  // Test counting number of arguments.
  emp_assert( EMP_COUNT_ARGS(a, b, c) == 3 );

  // Make sure EMP_STRINGIFY can process macros before running
  emp_assert( std::string(EMP_STRINGIFY(EMP_MERGE(ab, c, de, f))) == "abcdef" );
  emp_assert( std::string(EMP_STRINGIFY("abcdef")) == "\"abcdef\"" );

  // Make sure we can assemble arbitrary macros
  emp_assert( std::string(EMP_STRINGIFY( EMP_ASSEMBLE_MACRO( EMP_GET_ARG_, 2, x, y, z ))) == "y" );

  if (verbose) {
    std::cout << EMP_STRINGIFY( EMP_ASSEMBLE_MACRO( EMP_GET_ARG_, 2, x, y, z ) ) << std::endl;
    std::cout << EMP_STRINGIFY(EMP_MERGE(a, bc, d, e, f, g, h)) << std::endl;
  }

  // Make sure we can wrap each argument in a macro.
  emp_assert( std::string(EMP_STRINGIFY( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) )) == "[a] [b] [c] [d] [e] [f] [g] [h] [i] [j] [k] [l] [m] [n] [o] [p]" );
  emp_assert( EMP_STRINGIFY( EMP_LAYOUT(EMP_DECORATE, +, a, b, c, d, e, f, g, h) ) == "[a] + [b] + [c] + [d] + [e] + [f] + [g] + [h]" );

  if (verbose) {
    std::cout << EMP_STRINGIFY( EMP_WRAP_EACH(EMP_DECORATE, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) ) << std::endl;
    std::cout << EMP_STRINGIFY( EMP_LAYOUT(EMP_DECORATE, +, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z) ) << std::endl;
  }
}

