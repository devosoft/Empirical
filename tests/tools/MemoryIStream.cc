#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cstring>
#include <limits>
#include <ratio>

#include "tools/ContiguousStream.h"
#include "tools/MemoryIStream.h"
#include "tools/Random.h"
#include "tools/random_utils.h"

TEST_CASE("MemoryIStream", "[tools]") {

  const size_t num_ints = std::kilo{}.num;

  emp::Random rng{ 1 };
  emp::vector<int> source = emp::RandomVector<int>(
    rng,
    num_ints,
    std::numeric_limits<int>::min(),
    std::numeric_limits<int>::max()
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
