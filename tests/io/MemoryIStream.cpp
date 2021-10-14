/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file MemoryIStream.cpp
 */

#include <cstring>
#include <limits>
#include <ratio>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/io/ContiguousStream.hpp"
#include "emp/io/MemoryIStream.hpp"
#include "emp/math/Random.hpp"
#include "emp/math/random_utils.hpp"

TEST_CASE("MemoryIStream", "[io]") {

  const size_t num_ints = std::kilo{}.num;

  emp::Random rng{ 1 };
  emp::vector<int> source = emp::RandomVector<int>(
    rng,
    num_ints,
    0,
    std::mega{}.num
  );

  emp::MemoryIStream is{
    reinterpret_cast<const char*>( source.data() ),
    source.size() * sizeof(int)
  };

  emp::vector<char> dest(
    std::istreambuf_iterator<char>{is},
    {}
  );

  REQUIRE( 0 == std::memcmp(
    source.data(),
    dest.data(),
    dest.size()
  ) );

}
