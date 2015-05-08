#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/BitVector.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  const int set_size = 100000;
  typedef emp::BitVector TEST_TYPE;

  TEST_TYPE set1(set_size);
  TEST_TYPE set2(set_size);

  for (int i = 0; i < set_size; i++) {
    if (!(i%2) && (i%5)) set1[i] = 1;
    if (!(i%3) && (i%7)) set2.Set(i, true);
  }

  // TIMING!!!!!
  std::clock_t emp_start_time = std::clock();

  TEST_TYPE set3(set1 & set2);
  TEST_TYPE set4 = (set1 | set2);
  int total = 0;
  for (int i = 0; i < 100000; i++) {
    set3 |= (set4 << 3);
    set4 &= (set3 >> 3);
    auto set5 = set3 & set4;
    total += set5.CountOnes();
  }

  std::clock_t emp_tot_time = std::clock() - emp_start_time;
  std::cout << "Time: "
            << 1000.0 * ((double) emp_tot_time) / (double) CLOCKS_PER_SEC
            << " ms" << std::endl;
  // END TIMING!!!

  if (verbose) {
    std::cout << "emp::BitVector objects: " << std::endl
              << "   " << set1 << std::endl
              << "   " << set2 << std::endl
              << "   " << set3 << std::endl
              << "   " << set4 << std::endl
              << std::endl;
    ;
  }
}
