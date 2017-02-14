//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>
#include <string>

#include "../../base/assert.h"
#include "../../base/vector.h"
#include "../../config/command_line.h"
#include "../../tools/functions.h"
#include "../../tools/Random.h"


int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);
  const bool verbose = emp::cl::use_flag(args, "-v");

  int N = 15;
  int S = 1000;
  std::cout << N << std::endl;

  emp::Random random;

  for (int t = 0; t < N; t++) {
    std::string str;
    for (int i = 0; i < S; i++) str += (char) (random.GetInt(26) + 'A');

    std::cout << str << std::endl;

    for (int s=0; s<S; s++) {
      switch(random.GetInt(10)) {
      case 0:  // deletion!
        break;
      case 1:  // substitution!
        std::cout << (char) (random.GetInt(26) + 'A');
        break;
      case 2:  // insertion (1-3 chars..)!
        {
          int count = random.GetInt(3) + 1;
          for (int i = 0; i < count; i++) std::cout << (char) (random.GetInt(26) + 'A');
          // No break!  Still print original with insert.
        }
      default: // No changes.
        std::cout << str[s];
      }
    }
    std::cout << std::endl;
  }

  return 0;
}
