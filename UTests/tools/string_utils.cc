#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/string_utils.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  // TEST1: lets test our conversion to an escaped string.
  const std::string special_string = "This\t5tr1ng\nis\non THREE (3) \"lines\".";
  std::string escaped_string = emp::to_escaped_string(special_string);

  if (verbose) {
    std::cout << "Special string:" << std::endl << special_string << std::endl;
    std::cout << "Escaped string:" << std::endl << escaped_string << std::endl;
  }

  // note: we had to double-escape the test to make sure this worked.
  emp_assert(escaped_string == "This\\t5tr1ng\\nis\\non THREE (3) \\\"lines\\\".");

  // TEST2: Test more general conversion to literals.
  emp_assert(emp::to_literal(42) == "42");
  emp_assert(emp::to_literal('a') == "'a'");
  emp_assert(emp::to_literal('\t') == "'\\t'");
  emp_assert(emp::to_literal(1.234) == "1.234000");

  // TEST3: Make sure that we can properly identify different types of characters.
  int num_ws = 0;
  int num_cap = 0;
  int num_lower = 0;
  int num_let = 0;
  int num_num = 0;
  int num_alphanum = 0;
  for (char cur_char : special_string) {
    if (emp::is_whitespace(cur_char)) num_ws++;
    if (emp::is_upper_letter(cur_char)) num_cap++;
    if (emp::is_lower_letter(cur_char)) num_lower++;
    if (emp::is_letter(cur_char)) num_let++;
    if (emp::is_digit(cur_char)) num_num++;
    if (emp::is_alphanumeric(cur_char)) num_alphanum++;
  }
  int num_other = ((int) special_string.size()) - num_alphanum - num_ws;

  if (verbose) {
    std::cout << "num whitespace =   " << num_ws << std::endl
              << "num capitals =     " << num_cap << std::endl
              << "num lower lets =   " << num_lower << std::endl
              << "num letters =      " << num_let << std::endl
              << "num digits =       " << num_num << std::endl
              << "num alphanumeric = " << num_alphanum << std::endl
              << "num other chars =  " << num_other << std::endl;
  }

  emp_assert(num_ws == 6);
  emp_assert(num_cap == 6);
  emp_assert(num_lower == 16);
  emp_assert(num_let == 22);
  emp_assert(num_num == 3);
  emp_assert(num_alphanum == 25);
  emp_assert(num_other == 5);

  std::string base_string = "  This is   -MY-    string!!!!   ";
  

  return 0;
}
