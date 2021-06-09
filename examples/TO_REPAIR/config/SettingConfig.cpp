//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Some examples code for using emp::SettingConfig

#include <iostream>
#include "emp/config/SettingConfig.hpp"

#define PRINT(X) std::cout << #X " = " << X << '\n'

int main()
{
  emp::SettingConfig config_set;

  config_set.AddComboSetting<int>("int1") = { 1, 2, 3, 4 };
  config_set.AddComboSetting<std::string>("string") = { "a", "b", "cde" };
  config_set.AddComboSetting<int>("int2") = { 5 };
  config_set.AddComboSetting<double>("double", "A double value!", 'd') = { 1.1, 2.2 };

  do {
    std::cout << config_set.CurComboString() << '\n';
  } while (config_set.NextCombo());

  std::cout << "------------ Process Args ------------\n";

  auto out_args =
    config_set.ProcessOptions( { "testing", "--int1", "5,6,7", "unused", "-d", "3.3,4.4,5.5"} );

  do {
    std::cout << config_set.CurComboString() << '\n';
  } while (config_set.NextCombo());

  std::cout << "Remaining Args:\n";

  for (auto x : out_args) std::cout << "  " << x << '\n';

}
