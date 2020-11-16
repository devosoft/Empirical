// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is
// Copyright (C) Michigan State University, 2015. It is licensed
// under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <map>
#include <optional>

#include "emp/base/assert.hpp"
#include "emp/base/optional.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/config/config.hpp"
#include "assets/config_setup.hpp"

TEST_CASE("Test ArgManager", "[config]")
{

  // old ArgManager in cl namespace
  {

    emp::vector<std::string> arguments = {"command", "--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::cl::ArgManager am(argv.size() - 1, argv.data());

    REQUIRE(am.HasUnknown());

  }

  // minimal test
  {

    emp::vector<std::string> arguments = {"command", "--dir", "/some_path"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs();
    specs["dir"] = emp::ArgSpec(1);

    // Alternatively, we could merge in additional specifications from another unordered map:
    // std::unordered_map<std::string,emp::ArgSpec> additional_specs{"dir", emp::ArgSpec(1)};
    // specs.merge(additional_specs);

    // Another alternative, we can construct the key-value pair using emplace.
    // specs.try_emplace("dir", 1);

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      specs
    );

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/some_path"} );
    REQUIRE(!am.UseArg("dir"));

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"} );
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

  // more complicated test
  {

    MyConfig config;

    emp::vector<std::string> arguments = {
      "./command",
      "-unspecified",
      "unspec",
      "unspec",
      "-RANDOM_SEED",
      "32",
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
      "pos4",
      "--", // in POSIX, -- means treat subsequent words as literals
      "--duo",
      "-a",
      "b"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs(&config);
    specs["dir"] = emp::ArgSpec(
      1,
      "some information 'n stuff",
      std::unordered_set<std::string>{"d"}
    );
    specs["duo"] = emp::ArgSpec(2, "two things");
    specs["nope"] = emp::ArgSpec(0, "not here");

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      specs
    );

    am.PrintDiagnostic(std::cout);

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/some_path"} );
    REQUIRE(*am.UseArg("dir") == (emp::vector<std::string>) {"/other_path"} );
    REQUIRE(!am.UseArg("dir"));

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"./command"});

    REQUIRE(!am.ProcessBuiltin(&config));
    REQUIRE(!am.UseArg("help"));
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.UseArg("duo"));

    REQUIRE(
      *am.UseArg("_positional")
      == ((emp::vector<std::string>) {
        "pos1", "pos2", "pos3", "pos4", "--duo", "-a", "b"
      })
    );
    REQUIRE(!am.UseArg("_positional"));

    REQUIRE(!am.ProcessBuiltin(&config));
    REQUIRE(
      *am.UseArg("_unknown")
      == ((emp::vector<std::string>) {"-unspecified", "unspec", "unspec"})
    );
    REQUIRE(
      *am.UseArg("_unknown")
      == (emp::vector<std::string>) {"-unspecified"}
    );
    REQUIRE(*am.UseArg("_unknown") == ((emp::vector<std::string>) {"-a", "b"}));
    REQUIRE(!am.UseArg("_unknown"));

    REQUIRE(config.RANDOM_SEED() == 0);
    REQUIRE(am.ProcessBuiltin(&config));
    REQUIRE(config.RANDOM_SEED() == 32);

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

  // when no spec is provided (e.g., default builtins are used)
  {

    emp::vector<std::string> arguments = {
      "command", "--dir", "/some_path", "-unk", "-h"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::ArgManager am(argv.size() - 1, argv.data());

    am.PrintDiagnostic(std::cout);
    REQUIRE(
      *am.UseArg("_unknown")
      == ((emp::vector<std::string>) {"--dir", "/some_path"})
    );
    REQUIRE(*am.UseArg("_unknown") == (emp::vector<std::string>) {"-unk"});

    REQUIRE(am.UseArg("help") == (emp::vector<std::string>) {});

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

  // when empty spec is provided
  {

    emp::vector<std::string> arguments = {"command", "--dir", "/some", "-h"};

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      std::unordered_map<std::string, emp::ArgSpec>()
    );

    REQUIRE(am.HasUnused());

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(*am.UseArg("_unknown") == (emp::vector<std::string>) {"--dir"});
    REQUIRE(*am.UseArg("_unknown") == (emp::vector<std::string>) {"/some"});
    REQUIRE(*am.UseArg("_unknown") == (emp::vector<std::string>) {"-h"});

    REQUIRE(!am.HasUnused());

  }

  // test callbacks
  {

    MyConfig config;

    emp::vector<std::string> arguments = {
      "command",
      "-RANDOM_SEED",
      "32",
      "-no_callback",
      "--unknown"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs(&config);

    bool test = false;

    specs.merge(std::unordered_map<std::string,emp::ArgSpec>{
      {"no_callback", emp::ArgSpec(0, "no callback here!")},
      {"not_present", emp::ArgSpec(
        0,
        "blah",
        {},
        [&test](emp::optional<emp::vector<std::string>> res){
          if (!res) test = true;
        }
      )}
    });

    specs.erase("_unknown");

    emp::ArgManager am(
      argv.size() - 1,
      argv.data(),
      specs
    );

    REQUIRE(am.HasUnused());

    REQUIRE(config.RANDOM_SEED() == 0);
    REQUIRE(!test);
    am.UseCallbacks();
    REQUIRE(config.RANDOM_SEED() == 32);
    REQUIRE(config.RANDOM_SEED() == 32);
    REQUIRE(!am.UseArg("RANDOM_SEED"));
    REQUIRE(test);

    REQUIRE(*am.UseArg("_unknown") == (emp::vector<std::string>) {"--unknown"});

    REQUIRE(*am.UseArg("no_callback") == (emp::vector<std::string>) {});

    REQUIRE(!am.HasUnused());

  }

  // unit test for DealiasSpecs(spec_map_t inSpecsMap)
  {
    using spec_map_t = std::unordered_map<std::string, emp::ArgSpec>;
    spec_map_t iSpecs;
    iSpecs["noAlias"] = emp::ArgSpec(1, "test1");
    iSpecs["alias2a|alias2b"] = emp::ArgSpec(2, "test2");
    iSpecs["alias3a|alias3b|alias3c"] = emp::ArgSpec(3, "test3");

    auto oSpecs = emp::ArgManager::DealiasSpecs(iSpecs);

    auto expSpec1 = emp::ArgSpec(1, "test1");
    auto expSpec2 = emp::ArgSpec(2, "test2");
    expSpec2.aliases = {"alias2b"};
    auto expSpec3 = emp::ArgSpec(3, "test3");
    expSpec3.aliases = {"alias3b", "alias3c"};

    emp::ArgManager::spec_map_t expOSpecs;
    expOSpecs.emplace("noAlias", expSpec1);
    expOSpecs.emplace("alias2a", expSpec2);
    expOSpecs.emplace("alias3a", expSpec3);

    REQUIRE(oSpecs == expOSpecs);
  }

  // regression test (bug was positional arguments being interpreted as flags)
  {

    emp::vector<std::string> arguments = {
      "command", "help", "--halp"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    emp::ArgManager am(argv.size() - 1, argv.data());

    am.PrintHelp(std::cout);

    am.PrintDiagnostic(std::cout);

    REQUIRE(
      *am.UseArg("_positional")
      == ((emp::vector<std::string>) {"help"})
    );

    REQUIRE(
      *am.UseArg("_unknown")
      == ((emp::vector<std::string>) {"--halp"})
    );

    REQUIRE(am.UseArg("help") == std::nullopt);

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

  // test for single-letter options (commands) e.g., tar -czvf
  {

    emp::vector<std::string> arguments = {
      "command", "-ahi"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs();
    specs["apple|a"] = emp::ArgSpec(
      0,
      "some information 'n stuff"
    );
    specs["info|i"] = emp::ArgSpec(0, "no things");

    emp::ArgManager am(argv.size() - 1, argv.data(), specs);

    am.PrintHelp(std::cout);

    am.PrintDiagnostic(std::cout);

    REQUIRE(am.UseArg("help") == ((emp::vector<std::string>) {}));
    REQUIRE(am.UseArg("apple") == ((emp::vector<std::string>) {}));
    REQUIRE(am.UseArg("info") == ((emp::vector<std::string>) {}));

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

  // another test for single-letter options (commands) e.g., tar -czvf
  // do end strung-together single-letters w/ quotas > 0 up in _invalid?
  {

    emp::vector<std::string> arguments{
      "command", "-h", "-i", "boop", "-ahi"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs();
    specs["aardvark|a"] = emp::ArgSpec(
      0,
      "some information 'n stuff"
    );
    specs["ink|i"] = emp::ArgSpec(1, "a things");

    emp::ArgManager am(argv.size() - 1, argv.data(), specs);

    am.PrintHelp(std::cout);

    am.PrintDiagnostic(std::cout);

    REQUIRE(am.UseArg("help") == ((emp::vector<std::string>) {}));
    REQUIRE(am.UseArg("aardvark") == std::nullopt);
    REQUIRE(am.UseArg("ink") == ((emp::vector<std::string>) {"boop"}));

    REQUIRE(am.UseArg("_invalid") == ((emp::vector<std::string>) {"-ahi"}));

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

  // do we comply with POSIX ignore leading dashes after -- encountered?
  {

    emp::vector<std::string> arguments{
      "command", "--help", "--", "-i", "boop", "-ahi", "--aardvark"
    };

    std::vector<char*> argv;
    for (const auto& arg : arguments) argv.push_back((char*)arg.data());

    argv.push_back(nullptr);

    auto specs = emp::ArgManager::make_builtin_specs();
    specs["aardvark|a"] = emp::ArgSpec(
      0,
      "some information 'n stuff"
    );
    specs["ink|i"] = emp::ArgSpec(1, "a things");

    emp::ArgManager am(argv.size() - 1, argv.data(), specs);

    am.PrintHelp(std::cout);

    am.PrintDiagnostic(std::cout);

    REQUIRE(am.UseArg("help") == ((emp::vector<std::string>) {}));
    REQUIRE(am.UseArg("aardvark") == std::nullopt);
    REQUIRE(am.UseArg("ink") == std::nullopt);

    REQUIRE(am.UseArg("_positional") ==
      ((emp::vector<std::string>) {"-i", "boop", "-ahi", "--aardvark"})
    );

    REQUIRE(*am.UseArg("_command") == (emp::vector<std::string>) {"command"});
    REQUIRE(!am.UseArg("_command"));

    REQUIRE(!am.HasUnused());

  }

}
