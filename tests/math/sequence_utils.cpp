#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/math/sequence_utils.hpp"

#include <sstream>
#include <iostream>
#include <string>


TEST_CASE("Test sequence utils", "[math]")
{
  std::string s1 = "This is the first test string.";
  std::string s2 = "This is the second test string.";

  REQUIRE(emp::calc_hamming_distance(s1,s2) == 19);
  REQUIRE(emp::calc_edit_distance(s1,s2) == 6);

  // std::string s3 = "abcdefghijklmnopqrstuvwWxyz";
  // std::string s4 = "abBcdefghijXXmnopqrstuvwxyz";

  // std::string s3 = "lmnopqrstuv";
  // std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  std::string s3 = "adhlmnopqrstuvxy";
  std::string s4 = "abcdefghijklmnopqrstuvwxyz";

  emp::align(s3, s4, '_');

  REQUIRE(s3 == "a__d___h___lmnopqrstuv_xy_");

  emp::vector<int> v1 = { 1,2,3,4,5,6,7,8,9 };
  emp::vector<int> v2 = { 1,4,5,6,8 };

  emp::align(v1,v2,0);

  REQUIRE((v2 == emp::vector<int>({1,0,0,4,5,6,0,8,0})));
}
