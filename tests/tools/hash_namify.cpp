/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file hash_namify.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/tools/hash_namify.hpp"

TEST_CASE("Test hash_namify", "[tools]") {

  std::unordered_set< std::string > hash_names;

  for (size_t i{}; i < std::mega::num; ++i) {
    hash_names.insert( emp::hash_namify( i ) );
  }

  REQUIRE( hash_names.size() == std::mega::num );

}
