#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/functions.h"

char result_char;
void TestFun(int x, int y, char z) {
  result_char = z + x*y;
}

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

  emp_assert(emp::to_range(-10000, 10, 20) == 10);
  emp_assert(emp::to_range(9, 10, 20) == 10);
  emp_assert(emp::to_range(10, 10, 20) == 10);
  emp_assert(emp::to_range(11, 10, 20) == 11);
  emp_assert(emp::to_range(17, 10, 20) == 17);
  emp_assert(emp::to_range(20, 10, 20) == 20);
  emp_assert(emp::to_range(21, 10, 20) == 20);
  emp_assert(emp::to_range(12345678, 10, 20) == 20);
  emp_assert(emp::to_range<double>(12345678, 10, 20.1) == 20.1);
  emp_assert(emp::to_range(12345678.0, 10.7, 20.1) == 20.1);

  if (verbose) std::cout << "emp::to_range() passed test." << std::endl;

  
  // TEST FOR VARIADIC HELPER FUNCTIONS:

  emp_assert((emp::get_type_index<char, char, bool, int, double>()) == 0);
  emp_assert((emp::get_type_index<int, char, bool, int, double>()) == 2);
  emp_assert((emp::get_type_index<double, char, bool, int, double>()) == 3);
  emp_assert((emp::get_type_index<std::string, char, bool, int, double>()) < 0);

  if (verbose) std::cout << "emp::get_type_index() passed test." << std::endl;

  emp_assert((emp::has_unique_first_type<int, bool, std::string, bool, char>()) == true);
  emp_assert((emp::has_unique_first_type<bool, int, std::string, bool, char>()) == false);
  emp_assert((emp::has_unique_types<bool, int, std::string, std::vector<bool>, char>()) == true);
  emp_assert((emp::has_unique_types<int, bool, std::string, bool, char>()) == false);

  if (verbose) std::cout << "emp::has_unique*() passed test." << std::endl;


  std::tuple<int, int, char> test_tuple(3,2,'a');
  emp::ApplyTuple(TestFun, test_tuple);

  if (verbose) {
    std::cout << "Result of tuple application: " << result_char << std::endl;
  }
  
  emp_assert(result_char == 'g');

  return 0;
}
