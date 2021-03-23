// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <map>
#include <optional>
#include <fstream>

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
    config.Read("assets/test.cfg");

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 333);
    REQUIRE(config.TEST_STRING() == "default");

    config.RANDOM_SEED(123);

    std::cout << "Random seed = " << config.RANDOM_SEED() << std::endl;

    REQUIRE(config.RANDOM_SEED() == 123);

    config.WriteCSV("assets/test_new.csv");    
    std::ifstream fs1("assets/test.csv");
    std::ifstream fs2("assets/test_new.csv");
    std::string line1, line2;
    while(getline(fs1, line1))
    {
        getline(fs2, line2);
        REQUIRE(line1 == line2);
    }

    config.Write("assets/test_new.cfg");

    fs1.open("assets/test.cfg");
    fs2.open("assets/test_new.cfg");
    while(getline(fs1, line1))
    {
        getline(fs2, line2);
        REQUIRE(line1 == line2);
    }
  }

}
