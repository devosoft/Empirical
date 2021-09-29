//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2021.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  An example file for using StreamManager.hpp

#include <iostream>

#include "emp/io/StreamManager.hpp"

int main()
{
  emp::StreamManager sm;

  auto & test_stream = sm.GetOutputStream("standard out");
  test_stream << "Standard out works!" << std::endl;
}
