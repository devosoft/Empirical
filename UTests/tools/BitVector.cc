#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/BitVector.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::BitVector bv10(10);
  emp::BitVector bv32(32);
  emp::BitVector bv50(50);
  emp::BitVector bv64(64);
  emp::BitVector bv80(80);

  if (verbose) {
    std::cout << "Starting emp::BitVector objects: " << std::endl
              << "   " << bv10 << std::endl
              << "   " << bv32 << std::endl
              << "   " << bv50 << std::endl
              << "   " << bv64 << std::endl
              << "   " << bv80 << std::endl
      ;
      std::cout << std::endl;
  }

  bv80[70] = 1;
  emp::BitVector bv80c(bv80);

  if (verbose) std::cout << "Pre-shift:  " << bv80 << std::endl;
  bv80 <<= 1;
  // bv80 >>= 1;
  if (verbose) std::cout << "Post-shift: " << bv80 << std::endl;

  if (verbose) std::cout << "Shifting!" << std::endl;

  for (int i = 0; i < 75; i += 2) {
    emp::BitVector shift_vector = bv80 >> i;
    if (verbose) {
      if (i < 10) std::cout << ' ';
      std::cout << i << " : " << shift_vector
                << " : " << shift_vector.CountOnes()
                << " (" << (i <= 71) << ")"
                << std::endl;
    }

    emp_assert((shift_vector.CountOnes() == 1) == (i <= 71));
  }

  
  if (verbose) {
    std::cout << "Size 80 and its copy: " << std::endl
              << "   " << bv80 << std::endl
              << "   " << bv80c << std::endl
      ;
      std::cout << std::endl;
  }

  bv10 = (bv80 >> 70);
  if (verbose) std::cout << (bv80 >> 70) << std::endl;
  if (verbose) std::cout << bv10 << std::endl;

  // Test arbitrary bit retrieval of UInts
  bv80[65] = 1;
  emp_assert(bv80.GetUIntAtBit(64) == 130);
  emp_assert(bv80.GetValueAtBit<5>(64) == 2);  
}
