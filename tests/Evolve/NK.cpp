#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/Evolve/NK.hpp"

#include <sstream>
#include <iostream>

TEST_CASE("Test NK Landscapes", "[Evolve]")
{
	emp::NKLandscape nk0;
	REQUIRE(nk0.GetN() == 0);
	REQUIRE(nk0.GetK() == 0);

	emp::Random rnd(52);
	nk0.Config(5, 1, rnd);
	REQUIRE(nk0.GetStateCount() == 4);
	REQUIRE(nk0.GetTotalCount() == 20);
	nk0.SetState(0, 0, 1.0);
	nk0.SetState(1, 0, 1.0);
	nk0.SetState(2, 0, 1.0);
	nk0.SetState(3, 0, 2.0);
	nk0.SetState(4, 0, 1.0);
	emp::vector<size_t> states(5, 0);
	REQUIRE(nk0.GetFitness(states) == 6.0);
	emp::BitVector bv(5);
	REQUIRE(nk0.GetFitness(bv) == 6.0);
	REQUIRE(nk0.GetSiteFitness(3, bv) == 2);
	nk0.RandomizeStates(rnd, 100);
	REQUIRE(nk0.GetFitness(0, 0) != 1.0);

	emp::NKLandscapeMemo nk1(5, 1, rnd);
	REQUIRE(nk1.GetN() == 5);
	REQUIRE(nk1.GetK() == 1);
	emp::BitVector bv1(5);
	// fitness always equals these numbers because we seeded the Random object
	// REQUIRE( abs(nk1.GetFitness(0, bv1) - 0.363696309) < 0.01 );
	// REQUIRE( abs(nk1.GetFitness(bv1) - 2.695502239) < 0.01 );

	emp::NKLandscape nk2(9, 2, rnd);
	REQUIRE(nk2.GetN() == 9);
	REQUIRE(nk2.GetK() == 2);
	REQUIRE(nk2.GetStateCount() == 8);
	REQUIRE(nk2.GetTotalCount() == 72);
}
