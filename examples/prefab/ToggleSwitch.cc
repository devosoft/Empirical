//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "web/web.h"
#include "web/Div.h"
#include "web/Input.h"
#include "prefab/ToggleSwitch.h"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  UI::Input input_element(
      [](std::string str){;},
      "checkbox", NULL, "input_id"
  );
  emp::prefab::ToggleSwitch my_switch(input_element);
  doc << my_switch;

  UI::Div title;
  title << "Switch Label";
  my_switch.AddLabel(title);

  std::cout << "end of main... !" << std::endl;
}
