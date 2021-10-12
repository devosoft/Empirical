//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "emp/web/web.hpp"
#include "emp/prefab/ToggleSwitch.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

int main()
{
  emp::prefab::ToggleSwitch on_switch([](std::string val){}, "Switch Defult On", true, "user_defined_switch_id");
  doc << on_switch;

  doc << "<br>";

  emp::prefab::ToggleSwitch off_switch([](std::string val){}, NULL, false);
  doc << off_switch;
  off_switch.AddLabel("Switch Defult Off");

  std::cout << "end of main... !" << std::endl;
}
