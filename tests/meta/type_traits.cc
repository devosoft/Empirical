#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "meta/type_traits.h"

struct ExampleType1 {
  std::string ToString() { return "example"; }
  // double ToDouble() { return 42.0; }
};
struct ExampleType2 {
  // std::string ToString() { return "example"; }
  double ToDouble() { return 42.0; }
};

TEST_CASE("Test type_traits", "[games]")
{
  REQUIRE( emp::HasToString<ExampleType1>() == true );
  REQUIRE( emp::HasToString<ExampleType2>() == false );
  REQUIRE( emp::HasToString<std::string>() == false );
  REQUIRE( emp::HasToString<int>() == false );

  REQUIRE( emp::HasToDouble<ExampleType1>() == false );
  REQUIRE( emp::HasToDouble<ExampleType2>() == true );
  REQUIRE( emp::HasToDouble<std::string>() == false );
  REQUIRE( emp::HasToDouble<int>() == false );
}
