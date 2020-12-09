//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include <ratio>
#include <string>
#include <unordered_set>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/hash_namify.h"

TEST_CASE("Test hash_namify", "[tools]") {

  std::unordered_set< std::string > hash_names;

  for (size_t i{}; i < std::mega::num; ++i) {
    hash_names.insert( emp::hash_namify( i ) );
  }

  REQUIRE( hash_names.size() == std::mega::num );

}
