#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "tools/hash_utils.h"
#include "tools/vector_utils.h"

#include <sstream>
#include <iostream>
#include <unordered_set>

// // TODO: add asserts
// emp::Random grand;
TEST_CASE("Test hash_utils", "[tools]")
{
  {
  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);


  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)0) == (uint64_t)0);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)1) == (uint64_t)1);

  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)0) == (uint64_t)2);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)1) == (uint64_t)3);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)2) == (uint64_t)4);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)2) == (uint64_t)5);

  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)0) == (uint64_t)6);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)1) == (uint64_t)7);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)2) == (uint64_t)8);

  REQUIRE(emp::szudzik_hash((uint32_t)0, (uint32_t)3) == (uint64_t)9);
  REQUIRE(emp::szudzik_hash((uint32_t)1, (uint32_t)3) == (uint64_t)10);
  REQUIRE(emp::szudzik_hash((uint32_t)2, (uint32_t)3) == (uint64_t)11);

  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)0) == (uint64_t)12);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)1) == (uint64_t)13);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)2) == (uint64_t)14);
  REQUIRE(emp::szudzik_hash((uint32_t)3, (uint32_t)3) == (uint64_t)15);

  emp::vector<uint64_t> hash_vec;

  for(uint32_t i = 0; i < 10; ++i) {
    for(uint32_t j = 0; j < 10; ++j) {
      for(uint32_t s : { 0, 100, 100000 }) {
        hash_vec.push_back(emp::szudzik_hash(s+i,s+j));
      }
    }
  }

  std::unordered_set<uint64_t> hash_set(hash_vec.begin(), hash_vec.end());

  REQUIRE(hash_vec.size() == hash_set.size());
  }
  {
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 0) == (size_t) 0);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 1) == (size_t) 2654435769);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 2) == (size_t) 5308871538);
  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 0) == (size_t) 65);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 0) == (size_t) 130);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 1) == (size_t) 2654435832);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 2) == (size_t) 5308871664);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 3) == (size_t) 7963307496);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 3) == (size_t) 7963307370);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 1) == (size_t) 2654435962);
  }
}