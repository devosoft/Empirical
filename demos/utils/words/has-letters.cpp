//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2019.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Find all words in a dictionary with a combination of letters IN ORDER, annotated by length.

#include <fstream>
#include <iostream>
#include <string>

#include "../../../include/emp/base/vector.hpp"
#include "../../../include/emp/config/command_line.hpp"

// Test if a word has ALL of the required letters.
bool StringOK(const std::string & letters, const std::string & word) {
  size_t pos = 0;                                // Start at the begining of the word.
  for (char l : letters) {                       // Loop through all letters.
    pos = word.find(l, pos);                     // Find letter from current position.
    if (pos == std::string::npos) return false;  // If cur letter is not found, fail.
    pos++;                                       // Move on to the next position.
  }
  return true;
}

void Process(const std::string & letters, std::istream & is, std::ostream & os) {
  std::string cur_str;
  while (is) {
    is >> cur_str;
    if (StringOK(letters, cur_str)) {
      os << cur_str.size() << " " << cur_str << std::endl;
    }
  }
}

int main(int argc, char* argv[])
{
  emp::vector<std::string> args = emp::cl::args_to_strings(argc, argv);

  if (args.size() < 2 || args.size() > 4) {
    std::cerr << "Format: " << args[0] << " [letters] {input file} {output file}."
	      << std::endl;
    exit(1);
  }

  if (args.size() > 2) {
    std::ifstream is(args[2]);
    if (args.size() > 3) {
      std::ofstream os(args[3]);
      Process(args[1], is, os);
      os.close();
    } else {
      Process(args[1], is, std::cout);
    }
    is.close();
  }
  else {
    Process(args[1], std::cin, std::cout);
  }

}
