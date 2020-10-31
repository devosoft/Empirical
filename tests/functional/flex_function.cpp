#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/functional/flex_function.hpp"

#include <sstream>

TEST_CASE("Test flex_function", "[functional]")
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

int Sum4(int a1, int a2, int a3, int a4) {
  return a1 + a2 + a3 + a4;
}

TEST_CASE("Another Test flex_function", "[functional]")
{
  emp::flex_function<int(int,int,int,int)> ff = Sum4;
  ff.SetDefaults(10, 100,1000,10000);

  REQUIRE( ff(1,2,3,4) == 10 );
  REQUIRE( ff(1,2,3) == 10006 );
  REQUIRE( ff(1,2) == 11003 );
  REQUIRE( ff(1) == 11101 );
  REQUIRE( ff() == 11110 );
}
