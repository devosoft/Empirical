//  This file is part of Config Panel App
//  Copyright (C) Matthew Andres Moreno, 2020.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/config/command_line.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/web.hpp"
#include "emp/web/UrlParams.hpp"

#include "assets/SampleConfig.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");

Config cfg;

emp::prefab::ConfigPanel config_panel(cfg);

int main()
{
  // apply configuration query params and config files to Config
  auto specs = emp::ArgManager::make_builtin_specs(&cfg);
  emp::ArgManager am(emp::web::GetUrlParams(), specs);
  // cfg.Read("config.cfg");
  am.UseCallbacks();
  if (am.HasUnused()) std::exit(EXIT_FAILURE);

  // log configuraiton settings
  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  cfg.Write(std::cout);
  std::cout << "==============================\n" << std::endl;

  // setup configuration panel
  config_panel.Setup();
  doc << config_panel.GetConfigPanelDiv();

}
