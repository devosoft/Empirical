#define CATCH_CONFIG_MAIN
#define EMP_TRACK_MEM

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "tools/mem_track.h"

#include <sstream>
#include <string>

struct TestClass1 {
  TestClass1() {
    EMP_TRACK_CONSTRUCT(TestClass1);
  }
  ~TestClass1() {
    EMP_TRACK_DESTRUCT(TestClass1);
  }
};

struct TestClass2 {
  TestClass2() {
    EMP_TRACK_CONSTRUCT(TestClass2);
  }
  ~TestClass2() {
    EMP_TRACK_DESTRUCT(TestClass2);
  }
};

TEST_CASE("Test mem_track", "[tools]")
{
  emp::vector<TestClass1 *> test_v;
  TestClass2 class2_mem;

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 0);
  #endif

  for (int i = 0; i < 1000; i++) {
    test_v.push_back( new TestClass1 );
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 1000);
  #endif

  for (size_t i = 500; i < 1000; i++) {
    delete test_v[i];
  }

  #ifdef EMP_TRACK_MEM
  REQUIRE(EMP_TRACK_COUNT(TestClass1) == 500);
  //REQUIRE(EMP_TRACK_STATUS == 0);
  #endif

}
