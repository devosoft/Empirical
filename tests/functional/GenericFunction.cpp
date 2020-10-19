#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/functional/GenericFunction.hpp"

#include <sstream>

TEST_CASE("Test GenericFunction", "[functional]")
{
	emp::Function<double(double,double)> aFunction = [](double a, double b){ return a*b; };
	REQUIRE(aFunction(2.0,2.0) == 4.0);

	std::function<double(double,double)> anotherFunction = aFunction.GetFunction();
	REQUIRE(anotherFunction(5.0,5.0) == aFunction(5.0,5.0));

	REQUIRE(aFunction.CallOK<double>(2.0, 2.0));
	REQUIRE(!aFunction.CallOK<double>(5, "string"));

	REQUIRE( (aFunction.CallTypeOK<double, double, double>()) );
	REQUIRE( !(aFunction.CallTypeOK<double, std::string, int>()) );

	REQUIRE( (aFunction.ConvertOK<double(double,double)>()) );
}