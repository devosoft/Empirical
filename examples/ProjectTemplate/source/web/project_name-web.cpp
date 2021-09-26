/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file project_name-web.cpp
 */


//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

/*
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
