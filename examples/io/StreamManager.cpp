/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file StreamManager.cpp
 *  @brief An example file for using StreamManager.hpp
 */

#include <iostream>

#include "emp/io/StreamManager.hpp"

int main()
{
  emp::StreamManager sm;

  auto & test_stream = sm.GetOutputStream("standard out");
  test_stream << "Standard out works!" << std::endl;
}
