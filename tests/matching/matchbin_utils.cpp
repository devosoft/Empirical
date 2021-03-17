//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2020-2021.
//  Released under the MIT Software license; see doc/LICENSE

#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include "emp/data/DataNode.hpp"
#include "emp/matching/matchbin_utils.hpp"
#include "emp/matching/MatchBin.hpp"
#include "emp/math/Random.hpp"

#include <sstream>
#include <string>

// TODO break these tests up into multiple files

TEST_CASE("Test matchbin_utils", "[matchbin]")
{

    std::cout << "Checkpoint 1" << std::endl;

}
