#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "Evolve/Systematics.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Systematics", "[Evolve]")
{
	emp::Taxon<std::string> tx(0, "a");
	REQUIRE(tx.GetID() == 0);
	REQUIRE(tx.GetParent() == nullptr);
	REQUIRE(tx.GetInfo() == "a");
	REQUIRE(tx.GetNumOrgs() == 0);
	REQUIRE(tx.GetTotOrgs() == 0);
	tx.AddOrg();
	REQUIRE(tx.GetNumOrgs() == 1);
	tx.RemoveOrg();
	REQUIRE(tx.GetNumOrgs() == 0);
	REQUIRE(tx.GetTotOrgs() == 1);
	REQUIRE(tx.GetTotalOffspring() == 0);
	
	emp::Ptr< emp::Taxon<std::string, emp::datastruct::no_data> > parentPtr(&tx);
	emp::Taxon<std::string> tx_1(1, "b", parentPtr);
	REQUIRE(tx_1.GetParent() == parentPtr);
	tx_1.AddOffspring();
	REQUIRE(tx_1.GetTotalOffspring() == 1);
	REQUIRE(tx.GetTotalOffspring() == 1);
}