/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2024
*/
/**
 *  @file
 *  @brief Some example code for using emp::SudokuAnalyzer
 */

#include <iostream>

#include "emp/games/SudokuAnalyzer.hpp"

int main()
{
  emp::SudokuAnalyzer sa;
  sa.Load("sudoku_board-hard.dat");
  sa.Print();
  auto profile = sa.CalcProfile();
  std::cout << "============================================================================="
            << std::endl;
  profile.Print();
  sa.Print();
}
