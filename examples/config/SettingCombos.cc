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
}
