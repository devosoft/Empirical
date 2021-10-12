/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  EventDrivenGP.h
 *  @brief This is example code for using EventDrivenGP (SignalGP).
 */

 #include <iostream>

#include "emp/base/Ptr.hpp"
#include "emp/hardware/EventDrivenGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/hardware/EventLib.hpp"
#include "emp/math/Random.hpp"

// A few constants
constexpr int RANDOM_SEED = 1;
constexpr size_t HW_MAX_THREADS = 16;       // Maximum number of execution threads/'cores' that can be active at any one time.
constexpr size_t HW_MAX_CALL_DEPTH = 128;   // Maximum number of calls that can be active in the call stack at any one time.
constexpr double HW_MIN_SIM_THRESH = 0.0;   // Minimum similarity threshold required for a referring tag to match with a referrent.

// A few useful aliases:
using hardware_t = emp::EventDrivenGP_AW<16, emp::vector<double>>;  // SignalGP hardware with 16-bit tags.
                                               //   - Note that the SignalGP (emp::EventDrivenGP_AW) class
                                               //     is templated off of tag size (see hardware aliases above).
using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library
using event_lib_t = emp::EventLib<hardware_t>; // Event library

using inst_t = hardware_t::Instruction;
using event_t = hardware_t::Event;

// Some instruction implementations

// An instruction to roll a D20 die.
void Inst_RollD20(hardware_t & hw, const inst_t & inst) {
  hardware_t::State & state = hw.GetCurState();
  const size_t roll_val = hw.GetRandom().GetUInt(1, 21); // Generate a random number between (inclusive) 1 and 20.
  state.SetLocal(inst.args[0], roll_val); // Set local working memory to value of die roll.
}


int main() {
  // This example code is for the EventDrivenGP class. However, in the next iteration of EventDrivenGP, we will
  // rename EventDrivenGP to SignalGP. Additionally, we will update the terminology used within SignalGP.
  // In this example file, I will stick with the updated terminology.

  // ----------------------------------------------
  // --- Constructing SignalGP virtual hardware ---

  // As it is currently implemented, SignalGP requires 3 things on construction:
  //  1) A random number generator: emp::Random
  //    - The random number generator is used when breaking ties between tag-based references.
  //    - If you don't construct the hardware with a random number generator, it will construct it's own.
  //  2) An instruction library: emp::InstLib<HARDWARE_TYPE>
  //    - The instruction library contains all available instructions that can be executed on the virtual hardware and their implementations.
  //    - There is a default instruction library (and default instruction implementations) within the SignalGP hardware that you can use;
  //      however, I find it clearer to explicitly create a new instruction library for every experiment.
  //  3) An event library: emp::EventLib<HARDWARE_TYPE>
  //    - The event library contains events that can be triggered/handled by the virtual hardware.
  //    - There is a default event library (and default event handler implementations) within the SignalGP hardware; however,
  //      I find it clearer to explicitly create my event library for every experiment.
  //    - The default event library does not define dispatchers for events. That task is left up to the programmer.

  emp::Random random(RANDOM_SEED);  // Random number generator!
  inst_lib_t inst_lib;
  event_lib_t event_lib;

  // Let's make two SignalGP virtual hardwares: 2 with 16-bit tags
  hardware_t hw16_1(&inst_lib, &event_lib, &random);
  hardware_t hw16_2(&inst_lib, &event_lib, &random);

  // Spin up main cores (used to be handled in constructor)
  hw16_1.SpawnCore(0);
  hw16_2.SpawnCore(0);

  // Configure the hardware.
  hw16_1.SetMinBindThresh(HW_MIN_SIM_THRESH);
  hw16_1.SetMaxCores(HW_MAX_THREADS);
  hw16_1.SetMaxCallDepth(HW_MAX_CALL_DEPTH);

  hw16_2.SetMinBindThresh(HW_MIN_SIM_THRESH);
  hw16_2.SetMaxCores(HW_MAX_THREADS);
  hw16_2.SetMaxCallDepth(HW_MAX_CALL_DEPTH);

  // Create a way for the hardware to print our traits.
  auto trait_printer = [](std::ostream& os, emp::vector<double> traits){
      os << "[";
      if (traits.size()){
        for (unsigned int i = 0; i < traits.size() - 1; ++i){
          os << traits.at(i) << ", ";
        } os << traits[traits.size()-1];
      }
      os << "]";
    };
  hw16_1.SetTraitPrintFun(trait_printer);
  hw16_2.SetTraitPrintFun(trait_printer);


  // - We'll setup a hardware trait that will help us identify which hardware is which.
  const size_t TRAIT_IDX__ID = 0;

  hw16_1.GetTrait().push_back(1);
  hw16_2.GetTrait().push_back(2);

  hw16_1.GetTrait().push_back(4);
  hw16_1.PrintTraits();
  // -------------------------------------
  // --- Setting up an instruction set ---
  // Here's a list of all of the instructions with default implementations in the SignalGP class:
  //  - Inc, Dec, Not, Add, Sub, Mult, Div, Mod, TestEqu, TestNEqu, TestLess, If, While, Countdown,
  //    Close, Break, Call, Return, SetMem, CopyMem, SwapMem, Input, Output, Commit, Pull, BroadcastMsg,
  //    SendMsg, Fork, Terminate, Nop

  // Unless we want to do something different than the default for any of those instructions, we can start fleshing out
  // our instruction set by adding default instruction implementations.
  inst_lib.AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib.AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib.AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib.AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  inst_lib.AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  inst_lib.AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  inst_lib.AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  inst_lib.AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  inst_lib.AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  inst_lib.AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  inst_lib.AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
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
  // These next five instructions are 'block'-modifying instructions: they facilitate within-function flow control.
  // The {"block_def"} property tells the SignalGP virtual hardware that this instruction defines a new 'execution block'.
  // The {"block_close"} property tells the SignalGP virtual hardware that this instruction exits the current 'execution block'.
  inst_lib.AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib.AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib.AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");

  // What about some custom instructions?
  // Let's add an instruction to roll a 20-sided die.
  inst_lib.AddInst("RollD20", Inst_RollD20, 1, "WM[Arg1] = 20-sided die roll");

  // What about an instruction to allow the two SignalGP hardwares to communicate?
  // This time we'll use a lambda to specify how the instruction should work.
  inst_lib.AddInst("MsgFriend", [](hardware_t & hw, const inst_t & inst) {
    // Trigger a Msg event using the hardware that executed this instruction where the event's data is
    // the output memory of the sender.
    hw.TriggerEvent("Msg", inst.affinity, hw.GetCurState().output_mem);
  }, 0, "Send message to other SignalGP agent.");

  // -------------------------
  // --- Setting up events ---
  // Every event definition has three major components:
  // 1) A name (e.g. "Msg") - what is the event called?
  // 2) An event-handler - what happens when a hardware receives the event?
  // 3) A dispatcher - what happens when a hardware triggers the event?

  // When telling an event library about an event, we provide: the name, a function to handle the event, and a description.
  event_lib.AddEvent("Msg", [](hardware_t & hw, const event_t & event){
    // In the event handler, 'hw' is the SignalGP hardware *receiving* (handling) the event.
    hw.SpawnCore(event.affinity, hw.GetMinBindThresh(), event.msg); // To handle a message event, we'll spawn a new execution thread with the event's data (event.msg) as input.
  }, "Message event");

  // Wait, wait! We're not done with the Msg event. So far, we've specified its name and a handler.
  // We still need to specify what happens when a hardware triggers the event.
  event_lib.RegisterDispatchFun("Msg", [&hw16_1, &hw16_2](hardware_t & hw, const event_t & event){
    const size_t senderID = (size_t)hw.GetTrait()[TRAIT_IDX__ID]; // Who is sending/triggering (dispatching) this message?
    if (senderID == 1) { hw16_2.QueueEvent(event); }
    else { hw16_1.QueueEvent(event); }
  });

  // ----------------------------------------
  // --- Configuring programs on hardware ---
  // SignalGP programs are sets of functions where each function is a sequence of instructions.
  // Setup hardware 1's program:
  hw16_1.PushFunction();
  auto aff = hw16_1.GetProgram()[0].GetAffinity();
  aff.SetAll(); // Tag will be all ones
  hw16_1.GetProgram()[0].SetAffinity(aff);
  hw16_1.PushInst("RollD20", 0);
  hw16_1.PushInst("MsgFriend", 0, 0, 0, hardware_t::affinity_t());

  hw16_1.PushFunction();                        // Tag will be all zeros
  hw16_1.PushInst("Input", 0, 0);
  hw16_1.PushInst("Mult", 0, 0, 0);
  hw16_1.PushInst("Output", 0, 0);
  hw16_1.PushInst("MsgFriend", 0, 0, 0, hardware_t::affinity_t());

  // Setup hardware 2's program:
  hw16_2.PushFunction();
  hw16_2.PushInst("Input", 0, 0);
  hw16_2.PushInst("Inc", 0, 0, 0);
  hw16_2.PushInst("Inc", 0, 0, 0);
  hw16_2.PushInst("Output", 0, 0);
  hw16_2.PushInst("MsgFriend", 0, 0, 0, hardware_t::affinity_t());

  std::cout << "==== Hardware 1's Program ====" << std::endl;
  hw16_1.PrintProgramFull();
  std::cout << "==== Hardware 2's Program ====" << std::endl;
  hw16_2.PrintProgramFull();

  // ------------------------------------------------
  // --- Evaluating SignalGP programs on hardware ---
  std::cout << "----------------------------" << std::endl;
  std::cout << "--- Evaluating programs! ---" << std::endl;
  // Reset the hardware.
  hw16_1.ResetHardware();
  hw16_2.ResetHardware();
  // Important: we need to trigger a function on the hardware; otherwise,
  //            nothing will happen!
  hw16_1.SpawnCore(0);
  for (size_t t = 0; t < 32; ++t) {
    std::cout << "\n== Time: " << t << " ==" << std::endl;
    std::cout << "-- Hardware 1 --" << std::endl;
    hw16_1.PrintState();
    std::cout << "-- Hardware 2 --" << std::endl;
    hw16_2.PrintState();

    hw16_1.SingleProcess();
    hw16_2.SingleProcess();
  }
  std::cout << "\n== Time: FINAL ==" << std::endl;
  std::cout << "-- Hardware 1 --" << std::endl;
  hw16_1.PrintState();
  std::cout << "-- Hardware 2 --" << std::endl;
  hw16_2.PrintState();


  // ---------------------------------------------------
  // --- Generate random programs and evaluate them! ---
  const size_t min_fun_cnt = 1;
  const size_t max_fun_cnt = 16;
  const size_t max_arg_val = 16;
  const size_t min_fun_len = 1;
  const size_t max_fun_len = 32;

  for (size_t i = 0; i < 1000; ++i) {
    hardware_t::Program prog(&inst_lib);
    size_t fcnt = random.GetUInt(min_fun_cnt, max_fun_cnt+1);

    for (size_t fID = 0; fID < fcnt; ++fID) {
      hardware_t::Function new_fun;
      auto aff2 = new_fun.GetAffinity();
      aff2.Randomize(random);
      new_fun.SetAffinity(aff2);

      size_t icnt = random.GetUInt(min_fun_len, max_fun_len);

      for (size_t iID = 0; iID < icnt; ++iID) {
        new_fun.PushInst(random.GetUInt(prog.GetInstLib()->GetSize()),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              random.GetInt(max_arg_val),
                              hardware_t::affinity_t());
        new_fun.inst_seq.back().affinity.Randomize(random);
      }
      prog.PushFunction(new_fun);
    }

    std::cout << "==== Generated program: ====" << std::endl;
    hw16_1.Reset();         // Reset hardware, traits, and program.
    hw16_2.ResetHardware(); // HW1/HW2 can still communicate. We'll reset hw2 between program evaluations.
    hw16_1.SetProgram(prog);
    hw16_1.PrintProgramFull();
    for (size_t i = 0; i < 100; ++i) {
      hw16_1.SingleProcess();
      hw16_2.SingleProcess();
    }
  }
}
