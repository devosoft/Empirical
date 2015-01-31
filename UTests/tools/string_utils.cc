#include <iostream>
#include <string>
#include <vector>

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

  std::string base_string = "This is an okay string.\n  \tThis\nis   -MY-    very best string!!!!   ";

  std::string first_line = emp::string_pop_line(base_string);

  if (verbose) {
    std::cout << "First Line: " << std::endl
              << "\"" << first_line << "\"" << std::endl;
  }

  emp_assert(first_line == "This is an okay string.");
  emp_assert(emp::string_get_word(first_line) == "This");

  emp::string_pop_word(first_line);

  if (verbose) {
    std::cout << "Post-Pop first Line: " << std::endl
              << "\"" << first_line << "\"" << std::endl;
  }

  emp_assert(first_line == "is an okay string.");

  emp::remove_whitespace(first_line);
  
  if (verbose) {
    std::cout << "Post-remove whitespace: " << std::endl
              << "\"" << first_line << "\"" << std::endl;
  }

  emp_assert(first_line == "isanokaystring.");

  std::string popped_str = emp::string_pop(first_line, "ns");

  if (verbose) {
    std::cout << "Popped \"ns\": "
              << "\"" << popped_str << "\"" << std::endl;
  }

  emp_assert(popped_str == "i");
  emp_assert(first_line == "anokaystring.");



  popped_str = emp::string_pop(first_line, "ns");

  if (verbose) {
    std::cout << "Popped \"ns\": "
              << "\"" << popped_str << "\"" << std::endl;
  }

  emp_assert(popped_str == "a");
  emp_assert(first_line == "okaystring.");

  

  popped_str = emp::string_pop(first_line, 'y');

  if (verbose) {
    std::cout << "Popped 'y': "
              << "\"" << popped_str << "\"" << std::endl;
  }

  emp_assert(popped_str == "oka");
  emp_assert(first_line == "string.");

  

  
  if (verbose) {
    std::cout << "Second Line: " << std::endl
              << "\"" << base_string << "\"" << std::endl;
  }
  
  emp::left_justify(base_string);
  emp_assert(base_string == "This\nis   -MY-    very best string!!!!   ");

  emp::right_justify(base_string);
  emp_assert(base_string == "This\nis   -MY-    very best string!!!!");

  if (verbose) {
    std::cout << "Post-justification: " << std::endl
              << "\"" << base_string << "\"" << std::endl;
  }

  emp::compress_whitespace(base_string);
  emp_assert(base_string == "This is -MY- very best string!!!!");  

  if (verbose) {
    std::cout << "Post-compress: " << std::endl
              << "\"" << base_string << "\"" << std::endl;
  }

  std::vector<std::string> slices;
  emp::slice_string(base_string, slices, 's');

  if (verbose) {
    std::cout << "Slices:" << std::endl;
    for (std::string & cur_slice : slices) {
      std::cout << "- " << cur_slice << std::endl;
    }
  }

  emp_assert(slices.size() == 5);
  emp_assert(slices[1] == " i");
  emp_assert(slices[3] == "t ");

  return 0;
}
