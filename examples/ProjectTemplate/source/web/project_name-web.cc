//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

/*
 * WARNING: ProjectTemplate is deprecated in favor of the Cookiecutter Empirical
 * Project, which you can find at
 * https://github.com/devosoft/cookiecutter-empirical-project
*/

#include "web/web.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  doc << "<h1>Hello, world!</h1>";

  doc << "WARNING: ProjectTemplate is deprecated in favor of the Cookiecutter Empirical Project, which you can find at https://github.com/devosoft/cookiecutter-empirical-project";

}
