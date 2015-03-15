#include <iostream>
#include <string>
#include <vector>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/functions.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  int N;
  std::cin >> N;
  std::string in1, in2;
  for (int i = 0; i < N; i++) {
    std::cin >> in1 >> in2;
    std::cout << emp::calc_edit_distance(in1, in2) << std::endl;
  }

  return 0;
}
