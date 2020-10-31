//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataInterface demonstrating how to track different types of data.

#include <iostream>

#include "emp/data/DataInterface.hpp"

using data = emp::data;

int main()
{
  auto * di = emp::MakeDataInterface<double, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log>();



  delete di;
}
