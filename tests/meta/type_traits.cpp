#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/meta/type_traits.hpp"

struct ExampleType1 {
  std::string ToString() { return "example"; }
  // double ToDouble() { return 42.0; }
};
struct ExampleType2 {
  // std::string ToString() { return "example"; }
  double ToDouble() { return 42.0; }
};

TEST_CASE("Test type_traits", "[meta]")
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

TEST_CASE("Test is_streamable", "[meta]") {

  REQUIRE( emp::is_streamable<std::ostream, int>::value == true );
  REQUIRE( emp::is_streamable<std::ostream, std::string>::value == true );
  REQUIRE(
    emp::is_streamable<std::ostream, std::function<void()>>::value == false
  );

  REQUIRE( emp::is_streamable<std::stringstream, int>::value == true );
  REQUIRE( emp::is_streamable<std::stringstream, std::string>::value == true );
  REQUIRE(
    emp::is_streamable<std::stringstream, std::function<void()>>::value == false
  );

}
