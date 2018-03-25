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

  std::cout << "\nTrying out spreadsheet.  Initial:" << std::endl;
  emp::File spreadsheet;
  spreadsheet.Append("1,2,3");
  spreadsheet.Append("14,25,36");
  spreadsheet.Append("104,205,306,407,508");

  spreadsheet.Write(std::cout);

  emp::vector<std::string> first_col = spreadsheet.ExtractCol();
  std::cout << "\nAfter column is extracted:" << std::endl;
  spreadsheet.Write(std::cout);
  std::cout << "Extracted column: " << emp::to_string(first_col) << std::endl;

  emp::vector<size_t> second_col = spreadsheet.ExtractColAs<size_t>();
  std::cout << "\nAfter another column is extracted as size_t:" << std::endl;
  spreadsheet.Write(std::cout);
  std::cout << "Extracted column: " << emp::to_string(second_col) << std::endl;
  
}
