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
  CHECK(d2 == "123.000000");
  CHECK(d3 == "456.000000");
  CHECK(d4 == "789.000000");

  // Check string comparisons (d1 and d3)
  CHECK(d1 != "abc");
  CHECK(d3 != "");
  CHECK(d1 < "987");
  CHECK(d3 > "3");
  CHECK(d1 <= "987");
  CHECK(d3 >= "3");
  CHECK(d1 <= "789");
  CHECK(d3 >= "456");

  CHECK(d1 == d1);
  CHECK(d1 >= d1);
  CHECK(d1 <= d1);
  CHECK(d1 != d3);
  CHECK(d1 >= d3);
  CHECK(d1 > d3);
  CHECK(d3 != d1);
  CHECK(d3 <= d1);
  CHECK(d3 < d1);

  // Check number comparisons (d2 and d4)
  CHECK(d2 != 234);
  CHECK(d4 != 678);
  CHECK(d2 <  345);
  CHECK(d4 >  456.7);
  CHECK(d2 <= 789);
  CHECK(d4 >= 3.14);
  CHECK(d2 <= 123);
  CHECK(d4 >= 789);

  CHECK(d2 == d2);
  CHECK(d2 >= d2);
  CHECK(d2 <= d2);
  CHECK(d2 != d4);
  CHECK(d2 <= d4);
  CHECK(d2 < d4);
  CHECK(d4 != d2);
  CHECK(d4 >= d2);
  CHECK(d4 > d2);


  // Mixed comparisons
  CHECK(d1 > d2);
  CHECK(d1 >= d4);
  CHECK(d2 < d3);
  CHECK(d2 != "abc");
  CHECK(d2 != "234");
  CHECK(d2 < "234");
  CHECK(d2 <= "9");
  CHECK(d2 >= "0");
  CHECK(d2 > "0000000");
  
  CHECK(d3 != 234);
  CHECK(d3 < 789);
  CHECK(d3 <= 678);
  CHECK(d3 >= 234);
  CHECK(d3 > 0);
}
