// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/BitSet.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::BitSet<10> bs10;
  emp::BitSet<32> bs32;
  emp::BitSet<50> bs50;
  emp::BitSet<64> bs64;
  emp::BitSet<80> bs80;

  if (verbose) {
    std::cout << "Starting emp::BitSet objects: " << std::endl
              << "   " << bs10 << std::endl
              << "   " << bs32 << std::endl
              << "   " << bs50 << std::endl
              << "   " << bs64 << std::endl
              << "   " << bs80 << std::endl
      ;
      std::cout << std::endl;
  }

  bs80[70] = 1;
  emp::BitSet<80> bs80c(bs80);
  bs80 <<= 1;

  if (verbose) {
    std::cout << "Shifting!" << std::endl;
  }

  for (int i = 0; i < 75; i++) {
    emp::BitSet<80> shift_set = bs80 >> i;
    if (verbose) {
      if (i < 10) std::cout << ' ';
      std::cout << i << " : " << shift_set << std::endl;
    }

    emp_assert((shift_set.CountOnes() == 1) == (i <= 71));
  }

  
  if (verbose) {
    std::cout << "Size 80 and its copy: " << std::endl
              << "   " << bs80 << std::endl
              << "   " << bs80c << std::endl
      ;
      std::cout << std::endl;
  }

  // Test importing....
  bs10.Import(bs80 >> 70);

  if (verbose) {
    std::cout << (bs80 >> 70) << std::endl;
    std::cout << bs10 << std::endl;
  }

  emp_assert(bs10.GetUInt(0) == 2);


  // Test arbitrary bit retrieval of UInts
  bs80[65] = 1;
  emp_assert(bs80.GetUIntAtBit(64) == 130);
  emp_assert(bs80.GetValueAtBit<5>(64) == 2);
}
