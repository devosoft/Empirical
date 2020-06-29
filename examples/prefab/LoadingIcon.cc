//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "prefab/LoadingIcon.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  emp::prefab::LoadingIcon spinner;
  doc << spinner;
}
