/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file event_driven_gp.cpp
 */

#include <iostream>
#include <sstream>
#include <unordered_set>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

#include "emp/base/Ptr.hpp"
#include "emp/hardware/EventDrivenGP.hpp"
#include "emp/hardware/EventLib.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/hardware/signalgp_utils.hpp"
#include "emp/math/Random.hpp"

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
  std::string savedProgram = ss.str();  // save program for loading later
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

  // Event copy assignment operator
  auto ev2 = ev;
  gp.TriggerEvent(ev2);
  gp.PrintEvent(ev2, ss);
  REQUIRE(ss.str() == "[Message,00000000,(),(Properties: add)]");
  ss.str(std::string());

  // Event move assignment operator
  auto ev3 = std::move(ev2);
  gp.TriggerEvent(ev3);
  gp.PrintEvent(ev3, ss);
  REQUIRE(ss.str() == "[Message,00000000,(),(Properties: add)]");
  ss.str(std::string());

  // Event less-than operator
  emp::EventDrivenGP::Event ev4(1);
  REQUIRE(ev3 < ev4);
  emp::EventDrivenGP::Event ev5(1, {0, 1});
  REQUIRE(ev4 < ev5);

  // Event equals operator
  emp::EventDrivenGP::Event ev4_1(1);
  emp::EventDrivenGP::Event ev4_2(2);
  REQUIRE(ev4 == ev4_1);
  REQUIRE(!(ev4 == ev4_2));

  emp::EventDrivenGP::Event ev5_1(1, {0, 1});
  emp::EventDrivenGP::Event ev5_2(1, {1, 0});
  REQUIRE(ev5 == ev5_1);
  REQUIRE(!(ev5 == ev5_2));

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

TEST_CASE("Test SignalGP ('EventDrivenGP.h')", "[hardware]")
{
  // A few useful aliases:
  using hardware_t = emp::EventDrivenGP_AW<16, emp::vector<double>>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using event_lib_t = emp::EventLib<hardware_t>; // Event library
  using inst_t = hardware_t::Instruction;
  using event_t = hardware_t::Event;
  using tag_t = typename hardware_t::affinity_t;

  // A few constants
  constexpr size_t RANDOM_SEED = 1;
  constexpr double HW_MIN_SIM_THRESH = 0.5;
  constexpr size_t HW_MAX_THREADS = 32;
  constexpr size_t HW_MAX_CALL_DEPTH = 128;

  emp::Random random(RANDOM_SEED);
  inst_lib_t inst_lib;
  event_lib_t event_lib;

  // Let's make two SignalGP virtual hardwares: 2 with 16-bit tags
  hardware_t hw1(&inst_lib, &event_lib, &random);
  hardware_t hw2(&inst_lib, &event_lib, &random);

  // Spin up main cores (used to be handled in constructor)
  hw1.SpawnCore(0);
  hw2.SpawnCore(0);

  // Configure the hardware.
  hw1.SetMinBindThresh(HW_MIN_SIM_THRESH);
  hw1.SetMaxCores(HW_MAX_THREADS);
  hw1.SetMaxCallDepth(HW_MAX_CALL_DEPTH);
  REQUIRE(hw1.GetMinBindThresh() == HW_MIN_SIM_THRESH);
  REQUIRE(hw1.GetMaxCores() == HW_MAX_THREADS);
  REQUIRE(hw1.GetMaxCallDepth() == HW_MAX_CALL_DEPTH);

  hw2.SetMinBindThresh(HW_MIN_SIM_THRESH);
  hw2.SetMaxCores(HW_MAX_THREADS);
  hw2.SetMaxCallDepth(HW_MAX_CALL_DEPTH);
  REQUIRE(hw2.GetMinBindThresh() == HW_MIN_SIM_THRESH);
  REQUIRE(hw2.GetMaxCores() == HW_MAX_THREADS);
  REQUIRE(hw2.GetMaxCallDepth() == HW_MAX_CALL_DEPTH);

  hw1.GetTrait().resize(129);
  hw2.GetTrait().resize(1);
  const size_t TRAIT_IDX__ID = 0;
  hw1.GetTrait()[TRAIT_IDX__ID] = 1;
  hw1.GetTrait()[128] = -0.5;
  hw2.GetTrait()[TRAIT_IDX__ID] = 2;

  REQUIRE(hw1.GetTrait()[TRAIT_IDX__ID] == 1);
  REQUIRE(hw1.GetTrait()[128] == -0.5);
  REQUIRE(hw2.GetTrait()[TRAIT_IDX__ID] == 2);

  // Grab all of the default instructions.
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  REQUIRE(inst_lib.GetSize() == 1);

  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  REQUIRE(inst_lib.GetSize() == 2);

  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  inst_lib.AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  inst_lib.AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  inst_lib.AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  inst_lib.AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  inst_lib.AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  inst_lib.AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
  inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib.AddInst("CopyMem", hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  inst_lib.AddInst("SwapMem", hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  inst_lib.AddInst("Input", hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  inst_lib.AddInst("Output", hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  inst_lib.AddInst("Commit", hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  inst_lib.AddInst("Pull", hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  inst_lib.AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");
  inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
  inst_lib.AddInst("Terminate", hardware_t::Inst_Terminate, 0, "Kill current thread.");
  inst_lib.AddInst("RngDouble", hardware_t::Inst_RngDouble, 1, "Draw a double between 0 and 1 from onboard RNG.");
  inst_lib.AddInst(
    "Terminal",
    hardware_t::template Inst_Terminal<>,
    1,
    "Output an arbitrary but consistent value between 0 and 1."
  );

  // Add a simple MsgFriend instruction to facilitate communication between hw1 and hw2.
  inst_lib.AddInst("MsgFriend", [](hardware_t & hw, const inst_t & inst) {
    hw.TriggerEvent("Msg", inst.affinity, hw.GetCurState().output_mem);
  }, 0, "Send message to other SignalGP agent.");

  // Add Msg event definition to support hw1 <--> hw2 messaging.
  event_lib.AddEvent("Msg", [](hardware_t & hw, const event_t & event){
    hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg);
  }, "Message event");
  REQUIRE(event_lib.GetSize() == 1);

  // Wait, wait! We're not done with the Msg event. So far, we've specified its name and a handler.
  // We still need to specify what happens when a hardware triggers the event.
  event_lib.RegisterDispatchFun("Msg", [&hw1, &hw2](hardware_t & hw, const event_t & event){
    const size_t senderID = (size_t)hw.GetTrait()[TRAIT_IDX__ID];// Who is sending/triggering (dispatching) this message?
    if (senderID == 1) { hw2.QueueEvent(event); }
    else { hw1.QueueEvent(event); }
  });

  hw1.ResetProgram();
  hw2.ResetProgram();

  REQUIRE(hw1.GetProgram().GetSize() == 0);
  REQUIRE(hw2.GetProgram().GetSize() == 0);

  REQUIRE(hw1.GetTrait()[TRAIT_IDX__ID] == 1);
  REQUIRE(hw1.GetTrait()[128] == -0.5);
  REQUIRE(hw2.GetTrait()[TRAIT_IDX__ID] == 2);

  // Do a hard reset
  hw1.Reset();
  hw2.Reset();
  REQUIRE(hw1.GetProgram().GetSize() == 0);
  REQUIRE(hw2.GetProgram().GetSize() == 0);

  // Add handcoded program to hw1.
  hw1.PushFunction();
  auto aff = hw1.GetProgram()[0].GetAffinity();
  aff.SetAll();
  hw1.GetProgram()[0].SetAffinity(aff);
  hw1.PushInst("SetMem", 0, 16, 0, tag_t());
  hw1.PushInst("Commit", 0, 0);

  hw1.PushFunction();                        // Tag will be all zeros
  hw1.PushInst("Input", 0, 0);
  hw1.PushInst("While", 0);
  hw1.PushInst("Mult", 0, 0, 0);
  hw1.PushInst("Commit", 0, 0);
  hw1.PushInst("Close");

  REQUIRE(hw1.GetProgram().GetSize() == 2);
  REQUIRE(hw1.GetProgram()[0].GetSize() == 2);
  REQUIRE(hw1.GetProgram()[1].GetSize() == 5);

  // Run that handcoded program (w/function 0 as entry point)
  hw1.SpawnCore(0);
  hw1.Process(2);
  REQUIRE(hw1.GetShared(0) == 16);

  // Run handcoded program (w/function 1 as entry point)
  hw1.ResetHardware();
  REQUIRE(hw1.GetProgram().GetSize() == 2);

  hw1.SpawnCore(tag_t(), hw1.GetMinBindThresh(), {{0,2}}, false);
  hw1.Process(5);
  REQUIRE(hw1.GetShared(0) == 4);
  hw1.Process(4);
  REQUIRE(hw1.GetShared(0) == 16);

  // ---------------------------------------------------
  // --- Generate random programs and evaluate them! ---
  const size_t min_fun_cnt = 1;
  const size_t max_fun_cnt = 16;
  const size_t max_arg_val = 16;
  const size_t min_fun_len = 1;
  const size_t max_fun_len = 32;

  // Evaluate a bunch of randomly generated programs.
  for (size_t i = 0; i < 1000; ++i) {
    // Generate program for hardware 1
    hardware_t::Program prog1(&inst_lib);
    size_t fcnt = random.GetUInt(min_fun_cnt, max_fun_cnt+1);
    for (size_t fID = 0; fID < fcnt; ++fID) {
      hardware_t::Function new_fun;
      auto aff = new_fun.GetAffinity();
      aff.Randomize(random);
      new_fun.SetAffinity(aff);
      size_t icnt = random.GetUInt(min_fun_len, max_fun_len);
      for (size_t iID = 0; iID < icnt; ++iID) {
        new_fun.PushInst(random.GetUInt(prog1.GetInstLib()->GetSize()),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              hardware_t::affinity_t());
        new_fun.inst_seq.back().affinity.Randomize(random);
      }
      prog1.PushFunction(new_fun);
    }

    // Generate program for hardware 2
    hardware_t::Program prog2(&inst_lib);
    fcnt = random.GetUInt(min_fun_cnt, max_fun_cnt+1);
    for (size_t fID = 0; fID < fcnt; ++fID) {
      hardware_t::Function new_fun;
      auto aff = new_fun.GetAffinity();
      aff.Randomize(random);
      new_fun.SetAffinity(aff);

      size_t icnt = random.GetUInt(min_fun_len, max_fun_len);

      for (size_t iID = 0; iID < icnt; ++iID) {
        new_fun.PushInst(random.GetUInt(prog2.GetInstLib()->GetSize()),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              hardware_t::affinity_t());
        new_fun.inst_seq.back().affinity.Randomize(random);
      }
      prog2.PushFunction(new_fun);
    }

    hw1.GetTrait().resize(1);
    hw1.GetTrait()[TRAIT_IDX__ID] = 1;
    hw1.ResetProgram();
    REQUIRE(hw1.GetProgram().GetSize() == 0);
    REQUIRE(hw1.GetTrait()[TRAIT_IDX__ID] == 1);


    // Hard reset
    hw1.Reset();
    hw2.Reset();
    hw1.GetTrait().resize(1);
    hw2.GetTrait().resize(1);
    hw1.GetTrait()[TRAIT_IDX__ID] = 1;
    hw2.GetTrait()[TRAIT_IDX__ID] = 2;
    hw1.SetProgram(prog1);
    hw2.SetProgram(prog2);
    hw1.SpawnCore(tag_t(), hw1.GetMinBindThresh());
    hw2.SpawnCore(tag_t(), hw2.GetMinBindThresh());
    for (size_t t = 0; t < 128; ++t) {
      hw1.SingleProcess();
      hw2.SingleProcess();
    }
  }

}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: GenRandSignalGPTag", "[hardware]") {
  constexpr size_t RANDOM_SEED = 1;
  emp::Random random(RANDOM_SEED);

  std::unordered_set<uint32_t> uset; // Will be used to double-check uniqueness.

  // Generate a bunch of big random tags. No uniqueness guarantees.
  for (size_t i = 0; i < 100; ++i) auto tag = emp::GenRandSignalGPTag<1024>(random);

  // Enumerate all 2-bit tags
  emp::vector<emp::BitSet<2>> tags2;
  uset.clear();
  std::cout << "All two-bit tags: " << std::endl;
  for (size_t i = 0; i < emp::Pow2(2); ++i) {
    tags2.emplace_back(emp::GenRandSignalGPTag<2>(random, tags2));
    uset.emplace(tags2.back().GetUInt(0));
    std::cout << "  ";
    tags2.back().Print();
    std::cout << " : " << tags2.back().GetUInt(0) << std::endl;
  }
  REQUIRE(tags2.size() == emp::Pow2(2));
  REQUIRE(uset.size() == emp::Pow2(2));
  for (size_t i = 0; i < emp::Pow2(2); ++i) REQUIRE(emp::Has(uset, i));

  // Enumerate all 4-bit tags
  emp::vector<emp::BitSet<4>> tags4;
  uset.clear();
  std::cout << "All four-bit tags: " << std::endl;
  for (size_t i = 0; i < emp::Pow2(4); ++i) {
    tags4.emplace_back(emp::GenRandSignalGPTag<4>(random, tags4));
    uset.emplace(tags4.back().GetUInt(0));
    std::cout << "  ";
    tags4.back().Print();
    std::cout << " : " << tags4.back().GetUInt(0) << std::endl;
  }
  REQUIRE(tags4.size() == emp::Pow2(4));
  REQUIRE(uset.size() == emp::Pow2(4));
  for (size_t i = 0; i < emp::Pow2(4); ++i) REQUIRE(emp::Has(uset, i));

  // Generate a bunch of 16-bit tags.
  emp::vector<emp::BitSet<16>> tags16;
  for (size_t k = 0; k < 100; ++k) {
    uset.clear();
    tags16.clear();
    for (size_t i = 0; i < 1000; ++i) {
      tags16.emplace_back(emp::GenRandSignalGPTag<16>(random, tags16));
      uset.emplace(tags16.back().GetUInt(0));
    }
    REQUIRE(uset.size() == 1000);
  }
}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: GenRandSignalGPTags", "[hardware]") {
  constexpr size_t RANDOM_SEED = 1;
  emp::Random random(RANDOM_SEED);

  std::unordered_set<uint32_t> uset; // Will be used to double-check uniqueness.

  // Generate lots of small tags with no guarantees on uniqueness.
  auto small_tags = emp::GenRandSignalGPTags<2>(random, 1000);
  REQUIRE(small_tags.size() == 1000);
  // Generate lots of large tags with no guarantees on uniqueness.
  auto big_tags = emp::GenRandSignalGPTags<1024>(random, 1000);
  REQUIRE(big_tags.size() == 1000);

  // Use generator to enumerate all 2-bit tags.
  auto tags2 = emp::GenRandSignalGPTags<2>(random, emp::Pow2(2), true);
  uset.clear();
  for (size_t i = 0; i < tags2.size(); ++i) {
    uset.emplace(tags2[i].GetUInt(0));
  }
  REQUIRE(tags2.size() == emp::Pow2(2));
  REQUIRE(uset.size() == emp::Pow2(2));
  for (size_t i = 0; i < emp::Pow2(2); ++i) REQUIRE(emp::Has(uset, i));

  // Use generator to enumerate all 4-bit tags.
  auto tags4 = emp::GenRandSignalGPTags<4>(random, emp::Pow2(4), true);
  uset.clear();
  for (size_t i = 0; i < tags4.size(); ++i) {
    uset.emplace(tags4[i].GetUInt(0));
  }
  REQUIRE(tags4.size() == emp::Pow2(4));
  REQUIRE(uset.size() == emp::Pow2(4));
  for (size_t i = 0; i < emp::Pow2(4); ++i) REQUIRE(emp::Has(uset, i));

  // Generate a bunch of 8-bit tags (50% of the tag-space).
  // Check for uniqueness.
  auto tags8 = emp::GenRandSignalGPTags<8>(random, 128, true);
  uset.clear();
  for (size_t i = 0; i < tags8.size(); ++i) uset.emplace(tags8[i].GetUInt(0));
  REQUIRE(tags8.size() == 128);
  REQUIRE(uset.size() == 128);

  // Generate a bunch of 8-bit tags using previously generated tag as
  // reserved tag-space. Check for proper uniqueness.
  for (size_t i = 0; i < 100; ++i) {
    std::unordered_set<uint32_t> temp_set(uset);
    auto tags = emp::GenRandSignalGPTags<8>(random, 64, true, tags8);
    REQUIRE(tags.size() == 64);
    for (size_t t = 0; t < tags.size(); ++t) temp_set.emplace(tags[t].GetUInt(0));
    REQUIRE(temp_set.size() == 128+64);
  }

}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: GenRandSignalGPInst", "[hardware]") {
  using hardware_t = emp::EventDrivenGP_AW<16>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using inst_t = hardware_t::Instruction;

  constexpr int RANDOM_SEED = 1;
  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  emp::Random random(RANDOM_SEED);

  // Build a limited instruction library.
  inst_lib_t inst_lib;
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
  inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");

  inst_lib.PrintManifest();

  // Generate a bunch of random instructions, check that they conform with requested bounds.
  for (size_t i = 0; i < 10000; ++i) {
    inst_t inst(emp::GenRandSignalGPInst(random, inst_lib, MIN_ARG_VAL, MAX_ARG_VAL));
    REQUIRE(inst.args[0] >= MIN_ARG_VAL);
    REQUIRE(inst.args[0] <= MAX_ARG_VAL);
    REQUIRE(inst.args[1] >= MIN_ARG_VAL);
    REQUIRE(inst.args[1] <= MAX_ARG_VAL);
    REQUIRE(inst.args[2] >= MIN_ARG_VAL);
    REQUIRE(inst.args[2] <= MAX_ARG_VAL);
    REQUIRE(inst.id < inst_lib.GetSize());
  }
}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: GenRandSignalGPFunction", "[hardware]") {
  using hardware_t = emp::EventDrivenGP_AW<16>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using inst_t = typename hardware_t::Instruction;
  using fun_t = typename hardware_t::Function;

  constexpr int RANDOM_SEED = 1;

  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  constexpr size_t MIN_INST_CNT = 1;
  constexpr size_t MAX_INST_CNT = 32;

  emp::Random random(RANDOM_SEED);

  // Build a limited instruction library.
  inst_lib_t inst_lib;
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
  inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");

  // Generate a bunch of random functions.
  // Check that constraints weren't violated.
  for (size_t f = 0; f < 10000; ++f) {
    fun_t fun(emp::GenRandSignalGPFunction(random, inst_lib, MIN_INST_CNT, MAX_INST_CNT, MIN_ARG_VAL, MAX_ARG_VAL));
    REQUIRE(fun.GetSize() >= MIN_INST_CNT);
    REQUIRE(fun.GetSize() <= MAX_INST_CNT);
    for (size_t i = 0; i < fun.GetSize(); ++i) {
      inst_t & inst = fun[i];
      REQUIRE(inst.args[0] >= MIN_ARG_VAL);
      REQUIRE(inst.args[0] <= MAX_ARG_VAL);
      REQUIRE(inst.args[1] >= MIN_ARG_VAL);
      REQUIRE(inst.args[1] <= MAX_ARG_VAL);
      REQUIRE(inst.args[2] >= MIN_ARG_VAL);
      REQUIRE(inst.args[2] <= MAX_ARG_VAL);
      REQUIRE(inst.id < inst_lib.GetSize());
    }
  }
}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: GenRandSignalGPProgram", "[hardware]") {
  using hardware_t = emp::EventDrivenGP_AW<16, double>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using event_lib_t = emp::EventLib<hardware_t>;
  using inst_t = typename hardware_t::Instruction;
  using fun_t = typename hardware_t::Function;
  using program_t = typename hardware_t::Program;

  constexpr int RANDOM_SEED = 1;

  constexpr size_t MIN_ARG_VAL = 0;
  constexpr size_t MAX_ARG_VAL = 15;
  constexpr size_t MIN_INST_CNT = 1;
  constexpr size_t MAX_INST_CNT = 32;
  constexpr size_t MIN_FUN_CNT = 1;
  constexpr size_t MAX_FUN_CNT = 32;

  constexpr double HW_MIN_SIM_THRESH = 0.0;
  constexpr size_t HW_MAX_THREADS = 32;
  constexpr size_t HW_MAX_CALL_DEPTH = 128;

  emp::Random random(RANDOM_SEED);

  // Build a limited instruction library.
  inst_lib_t inst_lib;
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
  inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");

  // We'll use some SignalGP hardware to test randomly generated programs.
  event_lib_t event_lib;
  hardware_t hw(&inst_lib, &event_lib, &random);
  hw.SpawnCore(0); // Spin up main cores (used to be handled in constructor)
  hw.SetMinBindThresh(HW_MIN_SIM_THRESH);
  hw.SetMaxCores(HW_MAX_THREADS);
  hw.SetMaxCallDepth(HW_MAX_CALL_DEPTH);

  // Generate a bunch of random programs.
  // Check that constraints weren't violated.
  for (size_t p = 0; p < 1000; ++p) {
    program_t program(emp::GenRandSignalGPProgram(random, inst_lib, MIN_FUN_CNT, MAX_FUN_CNT, MIN_INST_CNT, MAX_INST_CNT, MIN_ARG_VAL, MAX_ARG_VAL));
    REQUIRE(program.GetSize() >= MIN_FUN_CNT);
    REQUIRE(program.GetSize() <= MAX_FUN_CNT);
    for (size_t f = 0; f < program.GetSize(); ++f) {
      fun_t & fun = program[f];
      REQUIRE(fun.GetSize() >= MIN_INST_CNT);
      REQUIRE(fun.GetSize() <= MAX_INST_CNT);
      for (size_t i = 0; i < fun.GetSize(); ++i) {
        inst_t & inst = fun[i];
        REQUIRE(inst.args[0] >= MIN_ARG_VAL);
        REQUIRE(inst.args[0] <= MAX_ARG_VAL);
        REQUIRE(inst.args[1] >= MIN_ARG_VAL);
        REQUIRE(inst.args[1] <= MAX_ARG_VAL);
        REQUIRE(inst.args[2] >= MIN_ARG_VAL);
        REQUIRE(inst.args[2] <= MAX_ARG_VAL);
        REQUIRE(inst.id < inst_lib.GetSize());
      }
    }
    // Run program on hardware.
    hw.Reset();
    hw.SetProgram(program);
    hw.Process(128);

    { // test with JSON archive
    std::stringstream stringstream;

    {
      cereal::JSONOutputArchive json_out(stringstream);
      json_out(program);
    }

    cereal::JSONInputArchive json_in(stringstream);

    program_t json_program(&inst_lib);
    json_in(json_program);

    REQUIRE(json_program == program);

    hw.Reset();
    hw.SetProgram(json_program);
    hw.Process(128);
    }

    { // test with binary archive
    std::stringstream stringstream;

    {
      cereal::BinaryOutputArchive binary_out(stringstream);
      binary_out(program);
    }

    cereal::BinaryInputArchive binary_in(stringstream);

    program_t binary_program(&inst_lib);
    binary_in(binary_program);

    REQUIRE(binary_program == program);

    hw.Reset();
    hw.SetProgram(binary_program);
    hw.Process(128);
    }


  }
}


TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: SignalGPMutator struct", "[hardware]") {
  using hardware_t = emp::EventDrivenGP_AW<16, double>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using program_t = typename hardware_t::Program;

  constexpr int RANDOM_SEED = 1;

  size_t MIN_ARG_VAL = 0;
  size_t MAX_ARG_VAL = 15;
  size_t MIN_FUNC_LEN = 1;
  size_t MAX_FUNC_LEN = 128;
  size_t MIN_FUNC_CNT = 1;
  size_t MAX_FUNC_CNT = 32;
  size_t MAX_TOTAL_LEN = 1024;

  emp::Random random(RANDOM_SEED);

  auto mutator = emp::SignalGPMutator<16>(MIN_FUNC_CNT, MAX_FUNC_CNT,
                                          MIN_FUNC_LEN, MAX_FUNC_LEN,
                                          MAX_TOTAL_LEN,
                                          MIN_ARG_VAL, MAX_ARG_VAL);

  // Build a limited instruction library.
  inst_lib_t inst_lib;
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib.AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.");
  inst_lib.AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib.AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib.AddInst("Fork", hardware_t::Inst_Fork, 0, "Fork a new thread. Local memory contents of callee are loaded into forked thread's input memory.");
  inst_lib.AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");

  // Check parameter adding.
  size_t default_param_cnt = mutator.GetParamCnt();
  size_t param1_id = mutator.AddParam("test1", 1.0, "Test parameter one!");
  size_t param2_id = mutator.AddParam("test2", 2.0, "Test parameter two!");
  size_t param3_id = mutator.AddParam("test3", 3.0, "Test parameter three!");
  REQUIRE(mutator.GetParamCnt() == default_param_cnt+3);
  REQUIRE(mutator.GetParam(param1_id) == 1.0);
  REQUIRE(mutator.GetParam(param1_id) == mutator.GetParam("test1"));
  REQUIRE(mutator.GetParamName(param1_id) == "test1");
  REQUIRE(mutator.GetParamDesc(param1_id) == "Test parameter one!");
  REQUIRE(mutator.GetParamID("test1") == param1_id);
  // Check parameter editing.
  mutator.SetParam(param1_id, 10.0);
  REQUIRE(mutator.GetParam(param1_id) == 10.0);
  mutator.SetParam(param1_id, 100.0);
  REQUIRE(mutator.GetParam("test1") == 100.0);

  // Add mutators.
  size_t default_mutator_cnt = mutator.GetMutatorCnt();
  mutator.ClearMutators();
  REQUIRE(mutator.GetMutatorCnt() == 0);
  mutator.AddMutator("99BottlesOfNothing", [](program_t &, emp::Random &)->size_t { return 99; }, "This mutator does nothing and returns 99.");
  mutator.AddMutator("AllFunTagsAllOnes", [](program_t & p, emp::Random & r)->size_t {
    for (size_t fID = 0; fID < p.GetSize(); ++fID) {
      auto aff = p[fID].GetAffinity();
      aff.SetAll();
      p[fID].SetAffinity(aff);
    }
    return p.GetSize();
  });

  // Generate a nop program to test custom mutators on.
  program_t nop_prog(&inst_lib);
  for (size_t f = 0; f < 3; ++f) {
    nop_prog.PushFunction();
    for (size_t i = 0; i < 8; ++i) nop_prog.PushInst("Nop");
  }
  size_t total_muts = mutator.ApplyMutations(nop_prog, random);
  REQUIRE(mutator.GetLastMutationCnt("99BottlesOfNothing") == 99);
  REQUIRE(mutator.GetLastMutationCnt("AllFunTagsAllOnes") == 3);
  REQUIRE(total_muts == 102);

  // Check removing a mutator.
  mutator.RemoveMutator("99BottlesOfNothing");
  REQUIRE(mutator.GetMutatorCnt() == 1);
  mutator.RemoveMutator("AllFunTagsAllOnes");
  REQUIRE(mutator.GetMutatorCnt() == 0);

  // Reset back to default.
  mutator.ResetMutators();
  REQUIRE(mutator.GetMutatorCnt() == default_mutator_cnt);
  // Crank up the mutation rates!
  mutator.SLIP__PER_FUNC(0.5);
  mutator.FUNC_DUP__PER_FUNC(0.5);
  mutator.FUNC_DEL__PER_FUNC(0.5);
  mutator.INST_INS__PER_INST(0.5);
  mutator.INST_DEL__PER_INST(0.5);
  mutator.ARG_SUB__PER_ARG(0.5);
  mutator.INST_SUB__PER_INST(0.5);
  mutator.TAG_BIT_FLIP__PER_BIT(0.5);
  // Generate many random programs, apply mutations, check constraints.
  for (size_t i = 0; i < 1000; ++i) {
    program_t prog(emp::GenRandSignalGPProgram(random, inst_lib,
                                              1, 8,
                                              mutator.GetProgMinFuncLen(), mutator.GetProgMaxFuncLen(),
                                              mutator.GetProgMinArgVal(), mutator.GetProgMaxArgVal()));
    for (size_t m = 0; m < 100; ++m) {
      mutator.ApplyMutations(prog, random);
      REQUIRE(mutator.VerifyProgram(prog));
    }
  }

  // Zero out all of the mutation rates.
  mutator.ARG_SUB__PER_ARG(0.0);
  mutator.INST_SUB__PER_INST(0.0);
  mutator.INST_INS__PER_INST(0.0);
  mutator.INST_DEL__PER_INST(0.0);
  mutator.SLIP__PER_FUNC(0.0);
  mutator.FUNC_DUP__PER_FUNC(0.0);
  mutator.FUNC_DEL__PER_FUNC(0.0);
  mutator.TAG_BIT_FLIP__PER_BIT(0.0);
  program_t prog1(emp::GenRandSignalGPProgram(random, inst_lib,
                                              mutator.GetProgMinFuncCnt(), mutator.GetProgMaxFuncCnt(),
                                              mutator.GetProgMinFuncLen(), mutator.GetProgMaxFuncLen(),
                                              mutator.GetProgMinArgVal(), mutator.GetProgMaxArgVal()));
  program_t prog2(prog1);
  mutator.ApplyMutations(prog1, random);
  REQUIRE(prog1 == prog2);

  // Check function duplications.
  mutator.FUNC_DUP__PER_FUNC(1.0);
  size_t orig_f_cnt = nop_prog.GetSize();
  mutator.ApplyMutations(nop_prog, random);
  REQUIRE(nop_prog.GetSize() == 2*orig_f_cnt);
  // Check function deletions.
  mutator.FUNC_DEL__PER_FUNC(1.0);
  mutator.FUNC_DUP__PER_FUNC(0.0);
  mutator.ApplyMutations(nop_prog, random);
  REQUIRE(nop_prog.GetSize() == mutator.GetProgMinFuncCnt());
}

TEST_CASE("Test SignalGP ('EventDrivenGP.h') utility: SignalGP MatchBin Cache", "[hardware]") {
  using hardware_t = emp::EventDrivenGP_AW<16>;
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
  using event_lib_t = emp::EventLib<hardware_t>; // Event library

  constexpr size_t RANDOM_SEED = 1;

  emp::Random random(RANDOM_SEED);
  inst_lib_t inst_lib;
  event_lib_t event_lib;
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");

  // Let's make two SignalGP virtual hardwares: 2 with 16-bit tags
  hardware_t hw1(&inst_lib, &event_lib, &random);
  hw1.SpawnCore(0); // Spin up main cores (used to be handled in constructor)

  REQUIRE(hw1.GetMatchBin().Size() == 0);

  // Add handcoded program to hw1.
  hw1.PushFunction();
  auto aff = hw1.GetProgram()[0].GetAffinity();
  aff.SetAll();
  hw1.GetProgram()[0].SetAffinity(aff);
  hw1.PushInst("Inc", 0);

  hw1.SetProgram(hw1.GetProgram());
  REQUIRE(hw1.GetMatchBin().Size() == 1);

  hw1.PushFunction();                        // Tag will be all zeros
  hw1.PushInst("Dec", 0);

  REQUIRE(hw1.GetMatchBin().Size() == 2);

  hw1.CallFunction(emp::BitSet<16>(), 0.5);
  REQUIRE(hw1.GetMatchBin().Size() == 2);

}
