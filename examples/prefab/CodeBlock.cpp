/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file CodeBlock.cpp
 */

#include <iostream>

#include "emp/prefab/CodeBlock.hpp"
#include "emp/web/web.hpp"

emp::web::Document doc("emp_base");

int main()
{
  std::cout << "entering main\n";
  std::string my_code =
  R"(
      int num = 9;
      std::cout << num << " is a square number" << std::endl;
  )";

  emp::prefab::CodeBlock code_block(my_code, "c++");
  doc << code_block;
}
