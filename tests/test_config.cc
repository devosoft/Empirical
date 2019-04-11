// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <iostream>

#include "base/assert.h"
#include "base/vector.h"
#include "config/ArgManager.h"
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

  emp::vector<std::string> arguments = {"--dir", "/some_path"};

  std::vector<char*> argv;
  for (const auto& arg : arguments) argv.push_back((char*)arg.data());

  argv.push_back(nullptr);

  {
  emp::cl::ArgManager am(argv.size() - 1, argv.data());
  }

  {
  emp::ArgManager am(argv.size() - 1, argv.data());
  }

  {
  emp::ArgManager am(
    argv.size() - 1,
    argv.data(),
    config.ArgCounts()
  );
  }

  {
  emp::ArgManager am(
    argv.size() - 1,
    argv.data(),
    config.ArgCounts(),
    config.ArgDescriptions()
  );
  }

}
