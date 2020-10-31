//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::SettingCombos

#include <iostream>
#include "emp/config/SettingCombos.hpp"

#define PRINT(X) std::cout << #X " = " << X << std::endl

int main()
{
  emp::SettingCombos config_set;

  config_set.AddSetting<int>("int1") = { 1, 2, 3, 4 };
  config_set.AddSetting<std::string>("string") = { "a", "b", "cde" };
  config_set.AddSetting<int>("int2") = { 5 };
  config_set.AddSetting<double>("double", "A double value!", 'd') = { 1.1, 2.2 };

  do {
    std::cout << config_set.CurString() << std::endl;
  } while (config_set.Next());

  std::cout << "------------ Process Args ------------\n";

  auto out_args =
    config_set.ProcessOptions( { "testing", "--int1", "5,6,7", "unused", "-d", "3.3,4.4,5.5"} );

  do {
    std::cout << config_set.CurString() << std::endl;
  } while (config_set.Next());

  std::cout << "Remaining Args:\n";

  for (auto x : out_args) std::cout << "  " << x << std::endl;

}
