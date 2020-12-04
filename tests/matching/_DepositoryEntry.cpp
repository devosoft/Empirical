//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/matching/_DepositoryEntry.hpp"

// stub test
TEST_CASE("DepositoryEntry", "[matching]") {
  emp::internal::DepositoryEntry<int, std::string, char>{{}, {}};
}
