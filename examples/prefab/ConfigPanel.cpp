/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file ConfigPanel.cpp
 */

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
