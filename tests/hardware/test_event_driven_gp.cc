#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch.hpp"

#include "hardware/EventDrivenGP.h"

#include <sstream>
#include <iostream>

TEST_CASE("Test Event Driven GP", "[Hardware]")
{
	// Event lib
	auto  HandleEvent_Message = [](
		emp::EventDrivenGP & hw,
		const emp::EventDrivenGP::Event & event
	) {
		hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg);
	};
	emp::EventDrivenGP::Instruction inst(0, 1); // increment id = 0, inc arg 0 = 1
	auto func = [inst](
		emp::EventDrivenGP & hd,
		const emp::EventDrivenGP::Event & ev
	){
		if (ev.HasProperty("add")) hd.ProcessInst(inst);
	};
	emp::EventLib<emp::EventDrivenGP> event_lib;
	event_lib.AddEvent("Message", HandleEvent_Message, "Event for exchanging messages (agent-agent, world-agent, etc.)");
	event_lib.RegisterDispatchFun(0, func);

	// Instruction lib
	auto  Inst_Inc = [](
		emp::EventDrivenGP & hw,
		const emp::EventDrivenGP::Instruction & inst
	) {
		emp::EventDrivenGP::State & state = hw.GetCurState();
		++state.AccessLocal(inst.args[0]);
	};
	auto Inst_Dec = [](
		emp::EventDrivenGP & hw,
		const emp::EventDrivenGP::Instruction & inst
	) {
		emp::EventDrivenGP::State & state = hw.GetCurState();
		--state.AccessLocal(inst.args[0]);
	};
	auto Inst_Not = [](
		emp::EventDrivenGP & hw,
		const emp::EventDrivenGP::Instruction & inst
	) {
		emp::EventDrivenGP::State & state = hw.GetCurState();
		state.SetLocal(inst.args[0], state.GetLocal(inst.args[0]) == 0.0);
	};
	emp::InstLib<emp::EventDrivenGP> inst_lib;
	inst_lib.AddInst(
		"Inc",
		Inst_Inc,
		1,
		"Increment value in local memory Arg1"
	);
	inst_lib.AddInst(
		"Dec",
		Inst_Dec,
		1,
		"Decrement value in local memory Arg1"
	);
	inst_lib.AddInst(
		"Not",
		Inst_Not,
		1,
		"Logically toggle value in local memory Arg1"
	);

	// Constructing EventDrivenGP
	emp::EventDrivenGP gp_default;
	REQUIRE( gp_default.GetInstLib() == gp_default.DefaultInstLib() );
	REQUIRE( gp_default.GetEventLib() == gp_default.DefaultEventLib() );
	emp::Random rnd(50);
	emp::Ptr<emp::Random> rndp(&rnd);
	emp::EventDrivenGP gp(inst_lib, event_lib, rndp);
	REQUIRE( (gp.GetRandom().GetSeed() == 50) );
	REQUIRE( gp.GetRandomPtr() == rndp );
	REQUIRE( gp.IsStochasticFunCall() );

	// Getters
	std::stringstream ss;
	emp::EventDrivenGP::Program c_prgm = gp.GetConstProgram();
	emp::EventDrivenGP::Program& prgm = gp.GetProgram();
	REQUIRE(c_prgm.GetSize() == 0);
	REQUIRE(prgm.GetSize() == 0);
	REQUIRE(gp.GetNumErrors() == 0);
	REQUIRE(gp.GetCurCoreID() == 0);
	REQUIRE(gp.GetCores().size() == gp.GetMaxCores());
	gp.PrintProgram(ss);
	REQUIRE(ss.str() == "");
	ss.str(std::string());

	// Pushing a function with an instruction
	emp::EventDrivenGP::Function fx;
	fx.PushInst(inst);
	gp.PushFunction(fx);
	REQUIRE(prgm.GetSize() == 1);
	REQUIRE(c_prgm.GetSize() == 0);
	REQUIRE(gp.GetFunction(0) == fx);
	gp.PrintProgramFull(ss);
	std::string savedProgram = ss.str();	// save program for loading later
	REQUIRE(savedProgram == "Fn-00000000:\n  Inc[00000000](1,0,0)\n\n");
	ss.str(std::string());

	// More instructions
	emp::EventDrivenGP::Instruction i_dec(1, 1); // decrement id = 1, dec arg 0 = 1
	gp.SetInst(0, 0, i_dec); // do same but just with inst params
	REQUIRE(gp.GetFunction(0)[0] == i_dec);
	gp.PrintInst(i_dec, ss);
	REQUIRE(ss.str() == "Dec 1");
	ss.str(std::string());

	// Testing states and SingleProcess
	gp.SpawnCore(0); // Spin up main core
	emp::EventDrivenGP::Instruction i_not(2, 0); // not id = 2, not arg 0 = 0
	gp.PushInst(i_not); // will be pushed onto the end of the last function
	REQUIRE(gp.GetFunction(0).GetSize() == 2);
	REQUIRE(gp.GetCurState().GetLocal(0) == 0.0);
	gp.CallFunction(0);
	REQUIRE(gp.GetCurState().GetLocal(1) == 0.0);
	gp.SingleProcess();
	REQUIRE(gp.GetCurState().GetLocal(1) == -1.0); // 1st inst is i_dec
	REQUIRE(gp.GetCurState().GetLocal(0) == 0.0);
	gp.SingleProcess();
	REQUIRE(gp.GetCurState().GetLocal(0) == 1.0); // 2nd inst is i_not
	gp.PrintState(ss);
	ss.str(std::string());

	// Testing default mem value
	REQUIRE(gp.GetCurState().GetLocal(1) == -1.0);
	gp.ProcessInst(inst); // will increment local mem 1
	REQUIRE(gp.GetCurState().GetLocal(1) == 0.0);
	REQUIRE(gp.GetDefaultMemValue() == 0.0);
	gp.SetDefaultMemValue(5.0);
	REQUIRE(gp.GetDefaultMemValue() == 5.0);
	gp.ProcessInst(inst);
	REQUIRE(gp.GetCurState().GetLocal(1) == 1.0);
	inst.Set(0, 2);
	gp.ProcessInst(inst);
	REQUIRE(gp.GetCurState().GetLocal(2) == 6.0);

	// New Random
	gp.NewRandom(150);
	(gp.GetRandom().GetSeed() == 150);

	// Events
	emp::EventDrivenGP::Event ev(0); // event 0 in event lib is message
	ev.properties.insert("add");
	gp.TriggerEvent(ev);
	REQUIRE(gp.GetCurState().GetLocal(1) == 2.0);
	gp.TriggerEvent(0); // doesn't have property add
	REQUIRE(gp.GetCurState().GetLocal(1) == 2.0);
	gp.PrintEvent(ev, ss);
	REQUIRE(ss.str() == "[Message,00000000,(),(Properties: add)]");
	ss.str(std::string());

	// Traits
	gp.GetTrait().push_back(2.0);
	REQUIRE(gp.GetTrait()[0] == 2.0);
	gp.GetTrait()[0] += 5.0;
	REQUIRE(gp.GetTrait()[0] == 7.0);
	gp.GetTrait()[0]--;
	REQUIRE(gp.GetTrait()[0] == 6.0);
	gp.GetTrait().push_back(3.0);
	REQUIRE(gp.GetTrait()[1] == 3.0);
        gp.SetTraitPrintFun(
          [](std::ostream& os, emp::vector<double> t){
            for (const auto & v : t) os << v << " ";
	  }
        );
	gp.PrintTraits(ss);
	REQUIRE(ss.str() == "6 3 ");
	ss.str(std::string());

	// Loading a program
	gp.PrintProgramFull(ss);
	REQUIRE(ss.str() == "Fn-00000000:\n  Dec[00000000](1,0,0)\n  Not[00000000](0,0,0)\n\n");
	ss.str(std::string());
	ss << savedProgram;
	gp.GetProgram().Load(ss);
	ss.str(std::string());
	std::stringstream os;
	gp.GetProgram().PrintProgramFull(os);
	REQUIRE(os.str() == savedProgram);
	os.str(std::string());
	REQUIRE(gp.GetProgram().GetSize() == 1);

	// Resetting the current state
	gp.GetCurState().Reset();
	for(size_t i=0;i<3;i++)
	{
		REQUIRE(gp.GetCurState().GetLocal(i) == gp.GetCurState().GetDefaultMemValue());
		REQUIRE(gp.GetCurState().GetLocalMemory()[i] == 0.0);
		REQUIRE(gp.GetCurState().GetInput(i) == gp.GetCurState().GetDefaultMemValue());
		REQUIRE(gp.GetCurState().GetInputMemory()[i] == 0.0);
		REQUIRE(gp.GetCurState().GetOutput(i) == gp.GetCurState().GetDefaultMemValue());
		REQUIRE(gp.GetCurState().GetOutputMemory()[i] == 0.0);
		REQUIRE(gp.GetCurState().AccessInput(i) == 0.0);
		REQUIRE(gp.GetCurState().AccessOutput(i) == 0.0);
		REQUIRE(gp.GetCurState().AccessLocal(i) == 0.0);
	}

	// Set/Push Inst
	REQUIRE(gp.GetFunction(0)[0].affinity == inst.affinity);
	REQUIRE(gp.GetFunction(0)[0].id == inst.id);
	gp.SetInst(0, 0, 1, 1);
	REQUIRE(gp.GetFunction(0)[0].affinity == inst.affinity);
	REQUIRE(gp.GetFunction(0)[0].id == 1);
	gp.PushInst(0, 0);
	REQUIRE(gp.GetFunction(0).GetSize() == 2);
	REQUIRE(gp.GetFunction(0)[1].id == 0);
	std::stringstream ss1;
	gp.PrintProgram(ss1);
	REQUIRE(ss1.str() == "Fn-0 00000000:\n  Dec 1\n  Inc 0\n\n");
	ss1.str(std::string());

	// Set Program
	REQUIRE(gp.GetProgram().GetSize() == 1);
	gp.SetProgram(c_prgm);
	REQUIRE(gp.GetProgram().GetSize() == 0);
	gp.SetShared(0, 2.5);
	REQUIRE(gp.GetShared(0) == 2.5);

	// Max cores
	gp.SetMaxCores(6);
	REQUIRE(gp.GetMaxCores() == 6);

	// StochasticFunCall
	gp.SetStochasticFunCall(false);
	REQUIRE(gp.IsStochasticFunCall() == false);

	// Copy constructor
	emp::EventDrivenGP gp2(gp);
	REQUIRE(gp2.GetMaxCores() == 6);
	REQUIRE(gp2.IsStochasticFunCall() == false);
	REQUIRE(gp2.GetDefaultMemValue() == 5.0);
	REQUIRE(gp2.GetSharedMem()[0] == 2.5);
}
