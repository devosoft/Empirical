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

  // old ArgManager in cl namespace
  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::cl::ArgManager am(argv.size() - 1, argv.data());

    REQUIRE(am.HasUnknown());

  }

  // minimal test
  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs(&config);
    specs.merge(
      (std::unordered_map<std::string,emp::ArgSpec>)
      { {"dir", emp::ArgSpec(1)} }
    );

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      specs
    );

    am.Print(std::cout);

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/some_path"} );
    REQUIRE(!am.UseArg("dir"));

    REQUIRE(!am.HasUnused());

  }

  // more complicated test
  {

    emp::vector<std::string> arguments = {
      "-unspecified",
      "unspec",
      "unspec",
      "--dir",
      "/some_path",
      "-d",
      "/other_path",
      "pos1",
      "pos2",
      "-unspecified",
      "-help",
      "pos3",
      "--duo",
      "b",
      "--duo",
      "-a",
      "b",
      "--duo",
      "a",
      "b",
      "pos4"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs(&config);

    specs.merge(std::unordered_map<std::string,emp::ArgSpec>{
      {"dir", emp::ArgSpec(1, "some information 'n stuff", {"d"})},
      {"duo", emp::ArgSpec(2, "two things")},
      {"nope", emp::ArgSpec(0, "not here")}
    });

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      specs
    );

    am.Print(std::cout);

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/some_path"} );
    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/other_path"} );
    REQUIRE(!am.UseArg("dir"));

    REQUIRE(!am.ProcessBuiltin());
    REQUIRE(!am.UseArg("help"));
    REQUIRE(am.ProcessBuiltin(&config));

    REQUIRE(!am.UseArg("duo"));

    REQUIRE(
      *am.UseArg("_positional")
      == ((emp::vector<std::string>) {"pos1", "pos2", "pos3", "pos4"})
    );
    REQUIRE(!am.UseArg("_positional"));

    REQUIRE(
      am.ViewArg("_unknown")
      == ((emp::vector<emp::vector<std::string>>) {
        {"-unspecified", "unspec", "unspec"},
        {"-unspecified"},
        {"-a", "b"}
      })
    );

    REQUIRE(
      am.ViewArg("duo")
      == ((emp::vector<emp::vector<std::string>>) {{"b"},{},{"a","b"}})
    );

    REQUIRE(am.ViewArg("nope") == (emp::vector<emp::vector<std::string>>) {});
    REQUIRE(
      am.ViewArg("extra_nope")
      == (emp::vector<emp::vector<std::string>>) {}
    );
    REQUIRE(!am.UseArg("nope"));
    REQUIRE(!am.UseArg("extra_nope"));

    REQUIRE(am.HasUnused());
  }

  // when no spec is provided
  {

    emp::vector<std::string> arguments = {"--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::ArgManager am(argv.size() - 1, argv.data());

    REQUIRE(am.HasUnused());

    REQUIRE(
      am.ViewArg("_unknown")
      == ((emp::vector<emp::vector<std::string>>) {{"--dir"}, {"/some_path"}})
    );

  }

}
