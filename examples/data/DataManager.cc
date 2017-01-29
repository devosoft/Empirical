//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Examples for DataNode demonstrating how to track different types of data.

#include <iostream>

#include "../../data/DataManager.h"

using data = emp::data;

int main()
{
  emp::DataManager<int, data::Current, data::Range, data::Pull, data::Log> dataM;
}
