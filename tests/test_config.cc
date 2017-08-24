// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is  
// Copyright (C) Michigan State University, 2015. It is licensed                
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <iostream>

#include "base/assert.h"
#include "config/command_line.h"
#include "config/config.h"
#include "config/config_setup.h"

TEST_CASE("Test config", "[config]"){
  MyConfig config;
  config.Read("config/test.cfg");

  std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

  REQUIRE(config.RANDOM_SEED() == 333);
  REQUIRE(config.TEST_STRING() == "default");

  config.RANDOM_SEED(123);

  std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

  REQUIRE(config.RANDOM_SEED() == 123);
}
