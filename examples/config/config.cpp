/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file config.cpp
 */

#include <iostream>

#include "config_setup.hpp"
#include "emp/config/config.hpp"

int main()
{
  MyConfig config;
  config.Write("test.cfg");
  config.WriteMacros("test-macro.h");
  std::cout << "Ping!" << std::endl;
}
