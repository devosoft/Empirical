/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2020
*/
/**
 *  @file
 */

#include <iostream>

#include "emp/prefab/LoadingIcon.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  emp::prefab::LoadingIcon spinner;
  doc << spinner;
}
