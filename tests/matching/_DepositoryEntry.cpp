/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file _DepositoryEntry.cpp
 */

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/matching/_DepositoryEntry.hpp"

// stub test
TEST_CASE("DepositoryEntry", "[matching]") {
  emp::internal::DepositoryEntry<int, std::string, char>{{}, {}};
}
