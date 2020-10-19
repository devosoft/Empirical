#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/scholar/Author.hpp"

TEST_CASE("Test Author", "[scholar]")
{
  emp::Author fml("First", "Middle", "Last");
  REQUIRE(fml.GetFullName() == "First Middle Last");

  emp::Author fl("No", "Mid");
  REQUIRE(fl.GetFullName() == "No Mid");

  emp::Author l("JustLast");
  REQUIRE(l.GetFullName() == "JustLast");

  emp::Author fml2("First", "Middiff", "Last");
  REQUIRE(fml2.GetFullName() == "First Middiff Last");
  REQUIRE(fml2 < fml);

  emp::Author auth1("aaa", "bbb", "ccc");
  emp::Author auth2("aaa", "bbb", "ccd");
  emp::Author auth3("aaa", "bbd", "ccc");
  emp::Author auth4("aad", "bbb", "ccc");
  emp::Author auth5("aaa", "bbb", "ccc");

  REQUIRE(auth1 < auth2);
  REQUIRE(auth1 < auth3);
  REQUIRE(auth1 < auth4);
  REQUIRE(auth1 == auth5);

  REQUIRE(auth2 > auth3);
  REQUIRE(auth2 > auth4);
  REQUIRE(auth2 > auth5);

  REQUIRE(auth3 < auth4);
  REQUIRE(auth3 > auth5);

  REQUIRE(auth4 > auth5);

  // Try out <= operator
  REQUIRE(auth1 <= auth2);
  REQUIRE(auth1 <= auth3);
  REQUIRE(auth1 <= auth4);
  REQUIRE(auth1 <= auth5);
}
