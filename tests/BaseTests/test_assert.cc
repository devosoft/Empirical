#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "base/assert.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test assert", "[base]")
{	
	// Best way to test different behavior of emp_assert based on compiler flags?

	emp_assert(false);
	REQUIRE(emp::assert_last_fail);
	
	/* not sure why file is assert.h and line_num is 95, is this expected behavior?
	std::string filen = emp::assert_fail_info.filename;
	std::string test_filen = "assert.h";
	REQUIRE(filen.substr(filen.size()-test_filen.size()) == test_filen);
	REQUIRE(emp::assert_fail_info.line_num == 95);
	*/
	
	REQUIRE(emp::assert_fail_info.error == "false");
	emp::assert_clear();
}