//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  An example file for using File.h

#include <iostream>

#include "tools/File.h"

int main()
{
  emp::File file("File.cc");
  std::cout << "File size = " << file.GetNumLines() << std::endl;
  file.RemoveEmpty();
  std::cout << "File size = " << file.GetNumLines() << std::endl;
  file.RemoveComments("//");   // Remove all C++ style comments.
  file.CompressWhitespace();
  file.Write(std::cout);
}
