//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Div.h"
#include "prefab/Collapse.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  UI::Div btn1;
  UI::Div content1;

  emp::prefab::Collapse collapse1(btn1, content1, true, "my_collapse");
  doc << collapse1.GetLinkDiv();
  doc << collapse1.GetToggleDiv();

  btn1.SetAttr("class", "btn btn-primary");
  btn1 << "Click me 1";
  content1 << "This content starts out open and has an id of 'my_collapse'.<hr>";
  content1 << "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

  doc << "<br><br>";
  
  UI::Div btn2;
  UI::Div content2;
  emp::prefab::Collapse collapse2(btn2, content2, false);
  doc << collapse2.GetLinkDiv();
  doc << collapse2.GetToggleDiv();

  btn2.SetAttr("class", "btn btn-primary");
  btn2 << "Click me 2";
  content2 << "This content starts out closed and has an emscripten generated id.<hr>";
  content2 << "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
  
}
