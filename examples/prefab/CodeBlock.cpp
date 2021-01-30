//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/web/web.hpp"
#include "emp/prefab/CodeBlock.hpp"

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
