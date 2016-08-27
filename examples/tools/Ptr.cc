//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some example code for using emp::Ptr

#include <iostream>
#include <string>

// EMP_TRACK_MEM must be defined in order to use the extra capabilities of emp::Ptr.
// Normally this would be done at the command line with -DEMP_TRACK_MEM
#define EMP_TRACK_MEM

#include "../../tools/Ptr.h"

int main()
{
  std::cout << "Testing!" << std::endl;
}
