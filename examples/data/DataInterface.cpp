/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file DataInterface.cpp
 *  @brief Examples for DataInterface demonstrating how to track different types of data.
 */

#include <iostream>

#include "emp/data/DataInterface.hpp"

using data = emp::data;

int main()
{
  auto * di = emp::MakeDataInterface<double, emp::data::Current, emp::data::Range, emp::data::Pull, emp::data::Log>();



  delete di;
}
