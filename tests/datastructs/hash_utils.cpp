#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"


#include <array>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include "emp/base/vector.hpp"
#include "emp/datastructs/hash_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"


// emp::Random grand;
TEST_CASE("Test hash_utils", "[datastructs]")
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
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 0) == (size_t) 2654435769);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 1) == (size_t) 2654435770);
  REQUIRE(emp::hash_combine((size_t) 0, (size_t) 2) == (size_t) 2654435771);
  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 0) == (size_t) 2654435832);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 0) == (size_t) 2654435899);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 1) == (size_t) 2654435835);
  REQUIRE(emp::hash_combine((size_t) 2, (size_t) 2) == (size_t) 2654435897);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 3) == (size_t) 2654435967);

  REQUIRE(emp::hash_combine((size_t) 1, (size_t) 3) == (size_t) 2654435837);
  REQUIRE(emp::hash_combine((size_t) 3, (size_t) 1) == (size_t) 2654435961);
  }

  {
    // this unordered map contains hash/key pairs generated by a known-good implementation of murmur3
    // since the 128-bit hashing function was used, these hash pairs only include the upper half of its output.
    // the known-good implementation was taken from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
    const std::unordered_map<
      uint64_t,
      std::vector<std::byte>
    > key_hashes{
      // test 1 byte hashes
        {5048724184180415669U, {std::byte{0x00}}}
      , {8849112093580131862U, {std::byte{0x01}}}
      , {7160176530259582706U, {std::byte{0x02}}}
      , {8244620721157455449U, {std::byte{0x03}}}
      , {10925832178609949229U, {std::byte{0x04}}}
      , {4214690439090310392U, {std::byte{0x05}}}
      , {1389283912212466035U, {std::byte{0x06}}}
      , {14134424961815854359U, {std::byte{0x07}}}
      , {16114901699465240702U, {std::byte{0x08}}}
      , {9228635489513802853U, {std::byte{0x09}}}
      , {3924702969362948632U, {std::byte{0x0A}}}
      , {10605915330328392167U, {std::byte{0x0B}}}
      , {17362459282200369457U, {std::byte{0x0C}}}
      , {10463378054868348838U, {std::byte{0x0D}}}
      , {6503564465305800252U, {std::byte{0x0E}}}
      , {1440240768098792060U, {std::byte{0x0F}}}
      , {7131323204957192209U, {std::byte{0xFA}}}

      // test some 2 byte hashes
      , {3478107235931676136U, {std::byte{0x00}, std::byte{0x00}}}
      , {11535399551474674108U, {std::byte{0x00}, std::byte{0xBC}}}
      , {13907197105396577022U, {std::byte{0xDC}, std::byte{0xFB}}}
      , {1159450886777878169U, {std::byte{0xBB}, std::byte{0xAE}}}
      , {2263528191020739987U, {std::byte{0xBE}, std::byte{0xEF}}}
      , {17029924217408412190U, {std::byte{0xCA}, std::byte{0xFE}}}

      // test 4 byte hashes, showing how transposition errors change the resulting hash significantly
      , {14710500545568337960U, {std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}, std::byte{0xDD}}}
      , {11132466383285019020U, {std::byte{0xBB}, std::byte{0xAA}, std::byte{0xCC}, std::byte{0xDD}}}
      , {6462300326891101032U, {std::byte{0xBB}, std::byte{0xAA}, std::byte{0xDD}, std::byte{0xCC}}}
      , {14376784789307187407U, {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}}}
      , {15197870503703975824U, {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x02}}}
      , {12969456943879063794U, {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x10}}}
      , {6487796989963411242U, {std::byte{0xDE}, std::byte{0xAD}, std::byte{0xBE}, std::byte{0xEF}}}

      // test 16 byte hashes with data from HashMetric tests
      , {594135684810436114U, {std::byte{188}, std::byte{118}, std::byte{76}, std::byte{216},
                               std::byte{221}, std::byte{247}, std::byte{160}, std::byte{207},
                               std::byte{196}, std::byte{240}, std::byte{27}, std::byte{8},
                               std::byte{50}, std::byte{217}, std::byte{74}, std::byte{129}}}
      , {12301005348282349488U, {std::byte{188}, std::byte{118}, std::byte{76}, std::byte{216},
                               std::byte{221}, std::byte{247}, std::byte{160}, std::byte{207},
                               std::byte{73}, std::byte{96}, std::byte{209}, std::byte{237},
                               std::byte{74}, std::byte{29}, std::byte{198}, std::byte{127}}}
      , {12729319182556432091U, {std::byte{188}, std::byte{118}, std::byte{76}, std::byte{216},
                               std::byte{221}, std::byte{247}, std::byte{160}, std::byte{207},
                               std::byte{1}, std::byte{122}, std::byte{48}, std::byte{84},
                               std::byte{144}, std::byte{72}, std::byte{213}, std::byte{15}}}
    };

    for (auto& [hash, data] : key_hashes) {
      // create temporal span to test
      std::span<const std::byte> spn(data);
      // make sure hashed value equals known-good value
      REQUIRE(hash == emp::murmur_hash(spn));
    }
  }
  // test ContainerHash
  {
    // we can create a hashing container with it
    std::unordered_map<
      emp::vector<int>,
      int,
      emp::ContainerHash<emp::vector<int>>
    > test_map{
        {{0, 0, 0, 0, 0, 0}, 0}
    };

    // make sure hashes stay the same
    const auto hasher1 = test_map.hash_function();

    REQUIRE(hasher1({0}) == 2654435769);
    REQUIRE(hasher1({0, 0}) == 175247769566);
    REQUIRE(hasher1({0, 0, 0}) == 11093822414574);
    REQUIRE(hasher1({1, 2, 3}) == 11093822460243);
    REQUIRE(hasher1({3, 2, 1}) == 11093822468169);
    REQUIRE(hasher1({0, 1, 2}) == 11093822415422);

    // we can also create a ContainerHash with a specific seed
    const auto hasher2 = emp::ContainerHash<emp::vector<int>, 28980>();

    // make sure hashes stay the same
    REQUIRE(hasher2({0}) == 2656277042);
    REQUIRE(hasher2({0, 0}) == 175101933815);
    REQUIRE(hasher2({0, 0, 0}) == 11084449574209);
    REQUIRE(hasher2({1, 2, 3}) == 11084449569853);
    REQUIRE(hasher2({3, 2, 1}) == 11084449921232);
    REQUIRE(hasher2({0, 1, 2}) == 11084449573900);
  }
}

