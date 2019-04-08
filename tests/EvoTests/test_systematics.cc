#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG

#include "third-party/Catch/single_include/catch.hpp"

#include "Evolve/SystematicsAnalysis.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Systematics", "[Evolve]")
{
	// Taxon
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
	
	// Systematics
	std::function<std::string(double &)> calc_taxon = [](double & o){ return o > 50.0 ? "large" : "small"; };
	emp::Systematics<double, std::string> sys1(calc_taxon);
	REQUIRE(sys1.GetTrackSynchronous() == false);
	REQUIRE(sys1.GetNumAncestors() == 0);
	REQUIRE(sys1.GetNumActive() == 0);
	REQUIRE(sys1.GetNumOutside() == 0);
	REQUIRE(sys1.GetTreeSize() == 0);
	REQUIRE(sys1.GetNumTaxa() == 0);
	
	sys1.SetTrackSynchronous(true);
	sys1.AddOrg(15.0, 0, 0, false);
	REQUIRE(sys1.GetNumActive() == 1);
	REQUIRE(sys1.GetTaxonAt(0)->GetInfo() == "small");
	sys1.AddOrg(56.0, 1, 0, true);
	REQUIRE(sys1.GetNumActive() == 2);
	REQUIRE(sys1.GetNextTaxonAt(1)->GetInfo() == "large");
	sys1.RemoveNextOrg(1);
	REQUIRE(sys1.GetNumActive() == 1);
	
	// Base setters and getters
	REQUIRE(sys1.GetStoreActive() == true);
	REQUIRE(sys1.GetStoreAncestors() == true);
	REQUIRE(sys1.GetStoreOutside() == false);
	REQUIRE(sys1.GetArchive() == true);
	REQUIRE(sys1.GetStorePosition() == true);
	sys1.SetStoreActive(false);
	REQUIRE(sys1.GetStoreActive() == false);
	sys1.SetStoreAncestors(false);
	REQUIRE(sys1.GetStoreAncestors() == false);
	sys1.SetStoreOutside(true);
	REQUIRE(sys1.GetStoreOutside() == true);
	sys1.SetArchive(false);
	REQUIRE(sys1.GetArchive() == false);
	sys1.SetStorePosition(false);
	REQUIRE(sys1.GetStorePosition() == false);
	
	#ifdef EMP_TDEBUG
	sys1.AddDeleteriousStepDataNodeImpl(true);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();
	
	sys1.AddVolatilityDataNodeImpl(true);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();
	
	sys1.AddUniqueTaxaDataNodeImpl(true);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();
	
	sys1.AddMutationCountDataNodeImpl(true);
	REQUIRE(emp::assert_last_fail);
	emp::assert_clear();
	#endif
	
	// Analysis
	using my_taxon = emp::Taxon<std::string, emp::datastruct::mut_landscape_info<double>>;
	//emp::Systematics<double, std::string, emp::datastruct::mut_landscape_info> sys2(calc_taxon)
	my_taxon taxon1(1, "medium");
	emp::Ptr<my_taxon> ptr1 = &taxon1;
	REQUIRE(emp::LineageLength(ptr1) == 1);
	my_taxon taxon2(1, "medium", ptr1);
	emp::Ptr<my_taxon> ptr2 = &taxon2;
	REQUIRE(emp::LineageLength(ptr1) == 1);
	REQUIRE(emp::LineageLength(ptr2) == 2);
	std::unordered_map<std::string, int> muts;
	muts["short"] = 12;
	muts["tall"] = 3;
	taxon2.GetData().RecordMutation(muts);
	REQUIRE(taxon2.GetData().mut_counts.size() == 2);
	REQUIRE(taxon2.GetData().mut_counts["tall"] == 3);
	
	emp::vector<std::string> types;
	types.push_back("tall");
	types.push_back("short");
	REQUIRE(emp::CountMuts(ptr2, types) == 15);
	REQUIRE(emp::CountMutSteps(ptr2, types) == 2);
	REQUIRE(emp::CountMutSteps(ptr2, "short") == 1);
	muts["short"] = 4;
	taxon1.GetData().RecordMutation(muts);
	REQUIRE(emp::CountMuts(ptr1, "short") == 4);
	REQUIRE(emp::CountMuts(ptr2, "short") == 16);
	REQUIRE(emp::CountMutSteps(ptr1, "short") == 1);
	REQUIRE(emp::CountMutSteps(ptr2, "short") == 2);
}