#include <iostream>
#include <string>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/sequence_utils.h"

char result_char;
void TestFun(int x, int y, char z) {
  result_char = z + x*y;
}

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  std::string s1 = "This is the first test string.";
  std::string s2 = "This is the second test string.";

  if (verbose) {
    std::cout << "String1: " << s1 << std::endl;
    std::cout << "String2: " << s2 << std::endl;
    std::cout << "Hamming: " << emp::calc_hamming_distance(s1,s2) << std::endl;
    std::cout << "Edit:    " << emp::calc_edit_distance(s1,s2) << std::endl << std::endl;
  }

  emp_assert(emp::calc_hamming_distance(s1,s2) == 19);
  emp_assert(emp::calc_edit_distance(s1,s2) == 6);
  
  // std::string s3 = "abcdefghijklmnopqrstuvwWxyz";
  // std::string s4 = "abBcdefghijXXmnopqrstuvwxyz";

  // std::string s3 = "lmnopqrstuv";
  // std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  std::string s3 = "adhlmnopqrstuvxy";
  std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  if (verbose) {
    std::cout << "Init 3:  " << s3 << std::endl;
    std::cout << "Init 4:  " << s4 << std::endl;
  }

  emp::align(s3, s4, '_');
  
  if (verbose) {
    std::cout << "Final 3: " << s3 << std::endl;
    std::cout << "Final 4: " << s4 << std::endl;
  }

  emp_assert(s3 == "a__d___h___lmnopqrstuv_xy_");

  std::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  std::vector<int> v2 = { 1,4,5,6,8 };

  if (verbose) {
    std::cout << "Init v1:  ";  for (int i : v1) std::cout << i << " "; std::cout << std::endl;
    std::cout << "Init v2:  ";  for (int i : v2) std::cout << i << " "; std::cout << std::endl;
  }

  emp::align(v1,v2,0);
  
  if (verbose) {
    std::cout << "Final v1: ";  for (int i : v1) std::cout << i << " "; std::cout << std::endl;
    std::cout << "Final v2: ";  for (int i : v2) std::cout << i << " "; std::cout << std::endl;
  }

  emp_assert((v2 == std::vector<int>({1,0,0,4,5,6,0,8,0})));

  return 0;
}
