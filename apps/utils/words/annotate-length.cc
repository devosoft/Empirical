//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Annotate all of the words in an input list with their length.

#include <iostream>
#include <set>

#include "../../../source/base/assert.h"
#include "../../../source/config/command_line.h"
#include "../../../source/tools/File.h"
#include "../../../source/tools/math.h"
#include "../../../source/tools/set_utils.h"
#include "../../../source/tools/string_utils.h"

void Process(std::istream & is, std::ostream & os) {
  std::string cur_str;
  while (is) {
    is >> cur_str;
    os << cur_str.size() << " " << cur_str << std::endl;
  }
}

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  if (args.size() > 3) {
    std::cerr << "Only a single input filename and output filename are allowed as arguments."
	      << std::endl;
    exit(1);
  }

  if (args.size() > 1) {
    std::ifstream is(args[1]);
    if (args.size() > 2) {
      std::ofstream os(args[2]);
      Process(is, os);
      os.close();
    } else {
      Process(is, std::cout);
    }
    is.close();
  }
  else {
    Process(std::cin, std::cout);
  }
	
}
