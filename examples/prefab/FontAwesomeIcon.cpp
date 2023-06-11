/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file FontAwesomeIcon.cpp
 */

#include <iostream>

#include "emp/prefab/FontAwesomeIcon.hpp"
#include "emp/web/Div.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  UI::Div toggleIcons;
  doc << toggleIcons;
  toggleIcons << "<h1>Collapse Icons</h1>";
  emp::prefab::FontAwesomeIcon up("fa-angle-double-up");
  toggleIcons << up;
  toggleIcons << "<br>";
  emp::prefab::FontAwesomeIcon down("fa-angle-double-down");
  toggleIcons << down;
  toggleIcons << "<br>";
  emp::prefab::FontAwesomeIcon right("fa-angle-double-right");
  toggleIcons << right;

  doc << "<br><hr><br>";

  UI::Div infoIcons;
  doc << infoIcons;
  infoIcons << "<h1>Circle icons</h1>";
  emp::prefab::FontAwesomeIcon i_circle("fa-info-circle");
  infoIcons << i_circle;
  infoIcons << "<br>";
  emp::prefab::FontAwesomeIcon question_circle("fa-question-circle-o");
  infoIcons << question_circle;
  infoIcons << "<br>";
  emp::prefab::FontAwesomeIcon plus_circle("fa-plus-circle");
  infoIcons << plus_circle;
}
