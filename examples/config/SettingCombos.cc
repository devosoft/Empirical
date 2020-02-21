//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::SettingCombos

#include <iostream>
#include "config/SettingCombos.h"

#define PRINT(X) std::cout << #X " = " << X << std::endl

int main()
{
  emp::SettingCombos config_set;

  config_set.AddSetting<int>("int1") = { 1, 2, 3, 4 };
  config_set.AddSetting<std::string>("string") = { "a", "b", "cde" };
  config_set.AddSetting<int>("int2") = { 5 };
  config_set.AddSetting<double>("double") = { 1.1, 2.2 };

  do {
    std::cout << config_set.CurString() << std::endl;
  } while (config_set.Next());
}
