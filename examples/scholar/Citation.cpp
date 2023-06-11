/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file Citation.cpp
 */

#include <iostream>

#include "emp/scholar/Citation.hpp"

int main()
{
  std::cout << "Testing..." << std::endl;
  emp::Citation cite1(emp::Citation::ARTICLE);
  emp::Author aut1("I", "Literate");
  cite1.SetTitle("This it the Best Title").SetJournal("The Best Journal").SetPages(100,200)
       .AddAuthor(aut1).AddAuthor("Me", "Myself", "I");
  cite1.Print();
}
