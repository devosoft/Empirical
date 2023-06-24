/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2023
*/
/**
 *  @file
 *
 * WARNING: the Cookiecutter Empirical Project, which you can find at
 * https://github.com/devosoft/cookiecutter-empirical-project, should be
 * preferred over ProjectTemplate
 */

#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Hello, world!</h1>";

  doc << "WARNING: the Cookiecutter Empirical Project, which you can find at https://github.com/devosoft/cookiecutter-empirical-project, should be preferred over ProjectTemplate";

}
