/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date ,020
 *
 *  @file ToggleSwitch.cpp
 */

#include <iostream>

#include "emp/prefab/ToggleSwitch.hpp"
#include "emp/web/web.hpp"

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
