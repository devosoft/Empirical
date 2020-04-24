#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/flex_function.h"

#include <sstream>

TEST_CASE("Test flex_function", "[tools]")
{
  emp::flex_function<int(int,int)> multiply_two;
  REQUIRE( !((bool) multiply_two) );
  multiply_two = [](int a, int b){ return a*b; };
  multiply_two.SetDefaults(0,0);
  REQUIRE((bool) multiply_two);
  REQUIRE((multiply_two() == 0));
  REQUIRE((multiply_two(5) == 0));
  REQUIRE((multiply_two(5,5) == 25));
  
  // TODO: test 'template <int ID> void SetDefault(pack_id<ID,ARGS...> & in_default)'
}