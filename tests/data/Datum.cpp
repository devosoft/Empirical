#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/data/Datum.hpp"


TEST_CASE("Test Datum", "[data]")
{

  emp::Datum d1;
  CHECK(d1.IsDouble() == true);

  emp::Datum d2("123");
  emp::Datum d3(456);
  d1 = "789";
  emp::Datum d4(d1);

  CHECK(d1.IsDouble() == false);
  CHECK(d2.IsDouble() == false);
  CHECK(d3.IsDouble() == true);
  CHECK(d4.IsDouble() == false);

  CHECK(d1 == 789);
  CHECK(d2 == 123);
  CHECK(d3 == 456);
  CHECK(d4 == 789);

  d2 = d2.AsDouble();
  d3 = d3.AsString();
  d4 = d4.AsDouble();

  CHECK(d1.IsString() == true);
  CHECK(d2.IsString() == false);
  CHECK(d3.IsString() == true);
  CHECK(d4.IsString() == false);

  CHECK(d1 == "789");
  CHECK(d2 == "123");
  CHECK(d3 == "456");
  CHECK(d4 == "789");
}
