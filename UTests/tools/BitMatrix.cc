#include <iostream>

#include "../../tools/assert.h"
#include "../../tools/command_line.h"
#include "../../tools/BitMatrix.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_arg(args, "-v");

  emp::BitMatrix<4,5> bm45;

  if (verbose) {
    std::cout << "Starting matrix: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }

  emp_assert(bm45.NumCols() == 4);
  emp_assert(bm45.NumRows() == 5);
  emp_assert(bm45.GetSize() == 20);

  if (verbose) std::cout << "Size info passed!" << std::endl;
  
  emp_assert(bm45.Any() == false);
  emp_assert(bm45.None() == true);
  emp_assert(bm45.All() == false);
  emp_assert(bm45.Get(1,1) == 0);
  emp_assert(bm45.Get(1,2) == 0);
  emp_assert(bm45.CountOnes() == 0);
  
  if (verbose) std::cout << "Empty tests passed." << std::endl;

  bm45.Set(1,2);  // Try setting a single bit!

  if (verbose) {
    std::cout << std::endl << "Turned a bit on at (1,2): " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }

  emp_assert(bm45.Any() == true);
  emp_assert(bm45.None() == false);
  emp_assert(bm45.All() == false);
  emp_assert(bm45.Get(1,1) == 0);
  emp_assert(bm45.Get(1,2) == 1);
  emp_assert(bm45.CountOnes() == 1);
  emp_assert(bm45.FindBit() == bm45.GetID(1,2));

  if (verbose) std::cout << "Single bit tests passed." << std::endl;

  bm45 = bm45.DownShift();

  if (verbose) {
    std::cout << std::endl << "Down shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.RightShift();

  if (verbose) {
    std::cout << std::endl << "Right shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.URShift();

  if (verbose) {
    std::cout << std::endl << "Upper-Right shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.UpShift();

  if (verbose) {
    std::cout << std::endl << "Up shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.ULShift();

  if (verbose) {
    std::cout << std::endl << "Upper-left shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.LeftShift();

  if (verbose) {
    std::cout << std::endl << "Left shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.DLShift();

  if (verbose) {
    std::cout << std::endl << "Down-Left shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
  bm45 = bm45.DRShift();

  if (verbose) {
    std::cout << std::endl << "Down-Right shifted: " << std::endl;
    bm45.Print();
    std::cout << std::endl;
  }
  
}
