#define CATCH_CONFIG_MAIN
#define EMP_TDEBUG

#include "third-party/Catch/single_include/catch.hpp"

#include "hardware/EventDrivenGP.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Event Driven GP", "[Hardware]")
{
	emp::Random rnd(50);
	emp::Ptr<emp::Random> rndp(&rnd);
	emp::EventDrivenGP gp(rndp);
	REQUIRE( (gp.GetInstLib() == gp.DefaultInstLib()) );
	REQUIRE( (gp.GetEventLib() == gp.DefaultEventLib()) );
	REQUIRE( (gp.GetRandom().GetSeed() == 50) );
	REQUIRE( gp.GetRandomPtr() == rndp );
	REQUIRE( gp.IsStochasticFunCall() );
	
	emp::EventDrivenGP::Program c_prgm = gp.GetConstProgram();
	emp::EventDrivenGP::Program& prgm = gp.GetProgram();
	REQUIRE(c_prgm.GetSize() == 0);
	REQUIRE(prgm.GetSize() == 0);
	REQUIRE(gp.GetNumErrors() == 0);
	REQUIRE(gp.GetCurCoreID() == 0);
	REQUIRE(gp.GetCores().size() == gp.GetMaxCores());
	
	emp::EventDrivenGP::Function fx;
	emp::EventDrivenGP::Instruction inst(0, 1); // increment id = 0, inc arg 0 = 1
	fx.PushInst(inst);
	gp.PushFunction(fx);
	REQUIRE(prgm.GetSize() == 1);
	REQUIRE(c_prgm.GetSize() == 0);
	REQUIRE(gp.GetFunction(0) == fx);
	
	emp::EventDrivenGP::Instruction i_dec(1, 1); // decrement id = 1, dec arg 0 = 1
	gp.SetInst(0, 0, i_dec);
	REQUIRE(gp.GetFunction(0)[0] == i_dec);
	
	REQUIRE(gp.GetCurState().GetLocal(1) == 0.0);
	gp.ProcessInst(inst); // will increment local mem 0
	REQUIRE(gp.GetCurState().GetLocal(1) == 1.0);
	REQUIRE(gp.GetDefaultMemValue() == 0.0);
	gp.SetDefaultMemValue(5.0);
	gp.ProcessInst(inst);
	REQUIRE(gp.GetCurState().GetLocal(1) == 2.0);
	inst.Set(0, 2);
	gp.ProcessInst(inst);
	REQUIRE(gp.GetCurState().GetLocal(2) == 6.0);
	
	gp.SetTrait(0, 2.0);
	REQUIRE(gp.GetTrait(0) == 2.0);
	gp.IncTrait(0, 5.0);
	REQUIRE(gp.GetTrait(0) == 7.0);
	gp.DecTrait(0);
	REQUIRE(gp.GetTrait(0) == 6.0);
	gp.PushTrait(3.0);
	REQUIRE(gp.GetTrait(1) == 3.0);
	
	gp.SetMaxCores(6);
	REQUIRE(gp.GetMaxCores() == 6);
	
	gp.SetStochasticFunCall(false);
	REQUIRE(gp.IsStochasticFunCall() == false);
}