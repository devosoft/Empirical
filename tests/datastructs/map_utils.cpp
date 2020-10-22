#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"

#include <sstream>
#include <iostream>
#include <string>


TEST_CASE("Test map_utils", "[datastructs]")
{
  std::map<int, char> test_map;
  test_map[0] = 'a';
  test_map[4] = 'e';
  test_map[8] = 'i';
  test_map[14] = 'o';
  test_map[20] = 'u';

  REQUIRE( emp::Has(test_map, 8) == true );
  REQUIRE( emp::Has(test_map, 18) == false );
  REQUIRE( emp::Find(test_map, 14, 'x') == 'o'); // 14 should be there as 'o'
  REQUIRE( emp::Find(test_map, 15, 'x') == 'x'); // 15 shouldn't be there, so return default.
  REQUIRE( emp::Has(test_map, 15) == false );    // Make sure 15 hasn't been added to the map.

  auto flipped = emp::flip_map(test_map);        // Make sure we can reverse the map.
  REQUIRE( emp::Has(flipped, 'u') == true);      // And the reversed map should have proper info.
  REQUIRE( emp::Has(flipped, 'x') == false);

  // Testing for bug #123
  std::map<std::string, std::string> test_123;
  test_123["1"] = "1";
  test_123["12"] = "12";

  REQUIRE( emp::Find(test_123, "0", "nothing") == "nothing" );
  REQUIRE( emp::Find(test_123, "1", "nothing") == "1" );
  REQUIRE( emp::FindRef(test_123, "1", "nothing") == "1" );

  // Test Keys
  emp::vector<int> key_v2 = emp::Keys(test_map);
  REQUIRE( emp::Has(key_v2, 0) );
  REQUIRE( emp::Has(key_v2, 4) );
  REQUIRE( emp::Has(key_v2, 8) );
  REQUIRE( emp::Has(key_v2, 14) );
  REQUIRE( emp::Has(key_v2, 20) );
  REQUIRE( key_v2.size() == 5 );
}
