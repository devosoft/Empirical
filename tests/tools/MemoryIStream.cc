#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cstring>
#include <ratio>

#include "tools/ContiguousStream.h"
#include "tools/MemoryIStream.h"
#include "tools/Random.h"

TEST_CASE("MemoryIStream", "[tools]") {

  const size_t num_ints = std::kilo{}.num;
  emp::vector<int> source(num_ints * sizeof(int));

  emp::Random rng{ 1 };
  rng.RandFill(
    reinterpret_cast<unsigned char*>( source.data() ),
    source.size() * sizeof(int)
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
