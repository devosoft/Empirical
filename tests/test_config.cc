// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <iostream>
#include <map>

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

  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::cl::ArgManager am(argv.size() - 1, argv.data());

  }

  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::ArgManager am(argv.size() - 1, argv.data());

  }

  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto counts = config.ArgCounts();
    counts.merge( (std::multimap<std::string,size_t>) { {"dir", 1} } );

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      counts
    );

  }

  {

    emp::vector<std::string> arguments = {
      "--dir",
      "/some_path",
      "--dir",
      "/other_path",
      "pos1",
      "pos2",
      "-help",
      "pos3",
      "--duo",
      "-a",
      "b",
      "pos4"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto counts = emp::ArgManager::MakeBuiltinCounts();
    counts.merge(config.ArgCounts());
    counts.merge( (std::multimap<std::string,size_t>) {
      {"dir", 1},
      {"duo", 2},
      {"nope", 0}
    });

    auto descs = emp::ArgManager::MakeBuiltinDescs();
    descs.merge(config.ArgDescriptions());
    descs.merge( (std::multimap<std::string,std::string>) {
      {"dir", "some information 'n stuff"},
      {"duo", "two things"},
      {"nope", "not here"}
    });

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      counts,
      descs
    );

    am.Print(std::cout);

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/some_path"} );
    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/other_path"} );
    REQUIRE(!am.UseArg("dir"));

    REQUIRE(!am.ProcessBuiltin());
    REQUIRE(!am.UseArg("help"));
    REQUIRE(am.ProcessBuiltin(&config));

    REQUIRE(*am.UseArg("duo") == ((emp::vector<std::string>) {"-a", "b"}));
    REQUIRE(!am.UseArg("duo"));

    REQUIRE(
      *am.UseArg("_positional")
      == ((emp::vector<std::string>) {"pos1", "pos2"})
    );
    REQUIRE(*am.UseArg("_positional") == (emp::vector<std::string>) {"pos3"} );
    REQUIRE(*am.UseArg("_positional") == (emp::vector<std::string>) {"pos4"} );
    REQUIRE(!am.UseArg("_positional"));

    REQUIRE(!am.HasUnused());
  }

}
