/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file errors.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/base/errors.hpp"

TEST_CASE("Test errors", "[base]")
{
  std::string excId = "exc1";
  emp::TriggerExcept(excId, "Exception one.");
  emp::ExceptInfo first = emp::GetExcept(excId);
  REQUIRE(first.id == excId);
  REQUIRE(first.desc == "Exception one.");
  REQUIRE(first.default_to_error);
  emp::ClearExcept(excId);
  REQUIRE(!emp::HasExcept());
  emp::ExceptInfo empty = emp::GetExcept(excId);
  REQUIRE(empty.id == "");
  REQUIRE(empty.desc == "");
  REQUIRE(empty.default_to_error == false);

  std::stringstream ss; // redirect cerr
  std::streambuf *old = std::cerr.rdbuf(ss.rdbuf());

  std::string error = "This is an error.";
  std::string warning = "This is a warning.";
  emp::NotifyWarning(warning);
  emp::NotifyError(error);
  emp::LibraryWarning(warning);
  emp::LibraryError(error);
  emp::InternalError(error);
  REQUIRE( ss.str() == "WARNING: This is a warning.\n"
            "ERROR: This is an error.\n"
            "EMPIRICAL USE WARNING: This is a warning.\n"
            "EMPIRICAL USE ERROR: This is an error.\n"
            "INTERNAL EMPIRICAL ERROR: This is an error.\n");
  std::cerr.rdbuf(old); // reset cerr
}

TEST_CASE("Another test errors", "[tools]")
{
  emp::TriggerExcept("test_fail", "The test failed.  *sob*");
  emp::TriggerExcept("test_fail2", "The second test failed too.  But it's not quite as aweful.", false);
  emp::TriggerExcept("test_fail2", "The third test is just test 2 again, but worse", true);

  REQUIRE( emp::CountExcepts() == 3 );
  auto except = emp::PopExcept("test_fail2");
  REQUIRE( emp::CountExcepts() == 2 );
  REQUIRE( except.desc == "The second test failed too.  But it's not quite as aweful." );
  REQUIRE( emp::HasExcept("test_fail2") == true );
  REQUIRE( emp::HasExcept("test_fail3") == false );
  emp::ClearExcepts();
  REQUIRE( emp::CountExcepts() == 0 );
}
