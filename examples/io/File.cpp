/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file File.cpp
 *  @brief An example file for using File.hpp
 */

#include <iostream>

#include "emp/io/File.hpp"

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
  spreadsheet.Append("1,2,3,4");
  spreadsheet.Append("14,25,36,47");
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

  emp::vector<std::string> first_row = spreadsheet.ExtractRow();
  std::cout << "\nAfter a row is extracted:" << std::endl;
  spreadsheet.Write(std::cout);
  std::cout << "Extracted row: " << emp::to_string(first_row) << std::endl;

  emp::vector<std::size_t> second_row = spreadsheet.ExtractRowAs<size_t>();
  std::cout << "\nAfter a row is extracted as size_t:" << std::endl;
  spreadsheet.Write(std::cout);
  std::cout << "Extracted row: " << emp::to_string(second_row) << std::endl;


  spreadsheet.Append("1000,1001,1002,1003");
  auto full_data = spreadsheet.ToData<size_t>();
  std::cout << "\nAfter all remaining data is extracted as size_t:" << std::endl;
  spreadsheet.Write(std::cout);
  std::cout << "Extracted data: " << emp::to_string(full_data) << std::endl;

}
