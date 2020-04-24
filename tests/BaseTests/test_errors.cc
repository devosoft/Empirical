#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "base/errors.h"

#include <sstream>
#include <iostream>

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