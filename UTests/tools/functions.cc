#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/functions.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  bool test_bool = true;
  emp::toggle(test_bool);
  emp_assert(test_bool == false);

  if (verbose) std::cout << "emp::toggle() passed test." << std::endl;

  emp_assert(emp::mod(10, 7) == 3);
  emp_assert(emp::mod(3, 7) == 3);
  emp_assert(emp::mod(-4, 7) == 3);
  emp_assert(emp::mod(-11, 7) == 3);

  if (verbose) std::cout << "emp::mod() passed test." << std::endl;

  emp_assert(emp::pow(2,3) == 8);
  emp_assert(emp::pow(-2,2) == 4);
  emp_assert(emp::pow(3,4) == 81);

  if (verbose) std::cout << "emp::pow() passed test." << std::endl;

  return 0;
}
