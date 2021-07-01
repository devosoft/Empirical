// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <map>
#include <optional>

#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"
#include "emp/config/config.hpp"
#include "assets/config_setup.hpp"

TEST_CASE("Test config", "[config]"){

  // test config class template
  {
    
    MyConfig config;

    // make sure printing/parsing do not alter config
    std::stringstream ss1, ss2;
    config.Write(ss1);
    config.Read(ss1);
    config.Write(ss2);
    std::string l1, l2;
    while ( ss1 && ss2 ) {
      std::getline(ss1, l1);
      std::getline(ss2, l2);
      // Check every line is the same between the "files"
      REQUIRE(l1 == l2);
    }


    config.Read("assets/test.cfg");

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 333);
    REQUIRE(config.TEST_STRING() == "default");

    config.RANDOM_SEED(123);

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 123);

  }

}
