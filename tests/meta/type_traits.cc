#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "meta/type_traits.h"

struct ExampleType1 {
  std::string ToString() { return "example"; }
};
struct ExampleType2 {
  // std::string ToString() { return "example"; }
};

TEST_CASE("Test type_traits", "[games]")
{
  REQUIRE( emp::HasToString<ExampleType1>() == true );
  REQUIRE( emp::HasToString<ExampleType2>() == false );
}
