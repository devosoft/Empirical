#define CATCH_CONFIG_MAIN
#include "third-party/Catch/single_include/catch.hpp"

#include <unordered_set>

#include "base/Ptr.h"
#include "hardware/EventDrivenGP.h"
#include "hardware/signalgp_utils.h"
#include "hardware/InstLib.h"
#include "hardware/EventLib.h"
#include "tools/Random.h"

TEST_CASE("Test SignalGP ('EventDrivenGP.h')", "[hardware]")
{
  // A few useful aliases: 
  using hardware_t = emp::EventDrivenGP_AW<16>;  // SignalGP hardware with 16-bit tags.
  using inst_lib_t = emp::InstLib<hardware_t>;   // Instruction library 
  using event_lib_t = emp::EventLib<hardware_t>; // Event library
  using inst_t = hardware_t::Instruction;
  using event_t = hardware_t::Event;
  using tag_t = typename hardware_t::affinity_t;

  // A few constants
  constexpr size_t RANDOM_SEED = 1;
  constexpr double HW_MIN_SIM_THRESH = 0.0;
  constexpr size_t HW_MAX_THREADS = 32;
  constexpr size_t HW_MAX_CALL_DEPTH = 128;
  
  emp::Random random(RANDOM_SEED);
  inst_lib_t inst_lib;
  event_lib_t event_lib;

  // Let's make two SignalGP virtual hardwares: 2 with 16-bit tags
  hardware_t hw1(&inst_lib, &event_lib, &random);
  hardware_t hw2(&inst_lib, &event_lib, &random);

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
  
  const size_t TRAIT_IDX__ID = 0;
  hw1.SetTrait(TRAIT_IDX__ID, 1);
  hw1.SetTrait(128, -0.5);
  hw2.SetTrait(TRAIT_IDX__ID, 2);

  REQUIRE(hw1.GetTrait(TRAIT_IDX__ID) == 1);
  REQUIRE(hw1.GetTrait(128) == -0.5);
  REQUIRE(hw2.GetTrait(TRAIT_IDX__ID) == 2);

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
    const size_t senderID = (size_t)hw.GetTrait(TRAIT_IDX__ID); // Who is sending/triggering (dispatching) this message?
    if (senderID == 1) { hw2.QueueEvent(event); } 
    else { hw1.QueueEvent(event); }
  });

  // Do a hard reset
  hw1.Reset();
  hw2.Reset();
  REQUIRE(hw1.GetProgram().GetSize() == 0);
  REQUIRE(hw2.GetProgram().GetSize() == 0);
  
  // Add handcoded program to hw1. 
  hw1.PushFunction();  
  hw1.GetProgram()[0].affinity.SetAll();
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
      new_fun.affinity.Randomize(random);
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
      new_fun.affinity.Randomize(random);

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
    // Hard reset
    hw1.Reset();
    hw2.Reset();
    hw1.SetTrait(TRAIT_IDX__ID, 1);
    hw2.SetTrait(TRAIT_IDX__ID, 2);
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