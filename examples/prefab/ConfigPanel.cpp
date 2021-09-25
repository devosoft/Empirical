//  This file is part of Config Panel App
//  Copyright (C) Matthew Andres Moreno, 2020.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"
#include "emp/web/web.hpp"

#include "assets/SampleConfig.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

Config cfg;

int main()
{
  // apply configuration query params and config files to Config
  auto specs = emp::ArgManager::make_builtin_specs(&cfg);
  emp::ArgManager am(emp::web::GetUrlParams(), specs);
  // cfg.Read("config.cfg");
  am.UseCallbacks();
  if (am.HasUnused()) std::exit(EXIT_FAILURE);
  emp::prefab::ConfigPanel config_panel(cfg);

  // log configuration settings
  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  cfg.Write(std::cout);
  std::cout << "==============================\n" << std::endl;

  // setup configuration panel
  doc << config_panel;

}
