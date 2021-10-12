// ------- DEPRECATED -------
// Example evolving EventDrivenGP.
// This example is a useful reference for how to use EventDrivenGP, but the fitness landscape is not particularly
// inspiring. Very harsh/noisy.
// Environment: Agents must accumulate resources by processing when resources are available. Any processing
//              done while resources are not available results in the accumulation of 'poison', which
//              is detrimental to agent fitness. To be optimal in this environment, agents must be
//              sure to only process resources when their available and to avoid processing when they
//              are unavailable.


#include <iostream>

#include "base/vector.h"
#include "base/Ptr.h"
#include "hardware/EventDrivenGP.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/BitSet.h"
#include "Evo/World.h"

using event_t = typename emp::EventDrivenGP::event_t;
using event_lib_t = typename emp::EventDrivenGP::event_lib_t;
using inst_t = typename emp::EventDrivenGP::inst_t;
using inst_lib_t = typename emp::EventDrivenGP::inst_lib_t;
using affinity_t = typename emp::EventDrivenGP::affinity_t;
using state_t = emp::EventDrivenGP::State;
using fun_t = emp::EventDrivenGP::Function;
using program_t = emp::EventDrivenGP::Program;
using hardware_t = emp::EventDrivenGP;
using memory_t = typename emp::EventDrivenGP::memory_t;

// ----- Configuration -----
constexpr size_t POP_SIZE = 1000;     // How big of a population are we working with?
constexpr size_t EVAL_TIME = 100;     // How much time (in CPU cycles) do we evaluate individuals before calculating fitness?
constexpr size_t GENERATIONS = 5000;  // How many generations (synchronous) are we running evolution?
constexpr int RAND_SEED = 1;         // Random seed?

constexpr size_t ENV_CHG_CHECK = 2;      // How often will the environment have the chance to change states?
constexpr double ENV_CHG_PERCENT = 0.10; // With what probability does the environment change when given the chance?

constexpr size_t MAX_FUNC_LENGTH = 32;  // How long are program functions allowed to be?
constexpr size_t MAX_FUNC_CNT = 4;      // How many functions are programs allowed to have?

constexpr size_t MAX_ARG_VAL = 16;      // Max argument value allowed to mutate in?
constexpr size_t MAX_HW_CORES = 4;      // How many cores do we allow hardware to run simultaneously?
constexpr size_t MAX_INST_ARGS = emp::EventDrivenGP::MAX_INST_ARGS;
constexpr double MIN_BIND_THRESH = 0.25;

// Virtual hardware trait IDs.
constexpr size_t TRAIT_ID__RES_SENSOR = 0;
constexpr size_t TRAIT_ID__RES_COLLECTED = 1;
constexpr size_t TRAIT_ID__POIS_COLLECTED = 2;

// Mutation rates
//  * Affinity Mutations    -- per-bit flips
constexpr double aff_bf_rate = 0.05;
//  * Substitution          -- per-copy/site instruction substitutions
constexpr double sub_rate = 0.005;
//  * Slip Mutations        -- per-function slip mutations
constexpr double slip_rate = 0.05;
//  * Function duplication  -- per-function whole-function duplications
constexpr double fun_dup_rate = 0.05;
//  * Function deletion     -- per-function whole-function deletion
constexpr double fun_del_rate = 0.05;

// Struct used to keep track of agents (target of evolution).
struct Agent {
  program_t program;
  size_t resources_collected;
  size_t poison_collected;

  Agent(const program_t & _p)
    : program(_p), resources_collected(0), poison_collected(0) { ; }

  program_t & GetGenome() { return program; }

  void ResetPhenotype() { resources_collected = 0; poison_collected = 0; }
};

void Inst_SenseResource(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  hw.TriggerEvent("SenseResource"); // Event dispatch should do the sensing.
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__RES_SENSOR));
}

void Inst_ProcessResource(emp::EventDrivenGP & hw, const inst_t & inst) {
  hw.TriggerEvent("ProcessResource");
}

void HandleEvent_SenseResource(emp::EventDrivenGP & hw, const event_t & event) {
  // Hardware doesn't need to do anything to handle this event (everything handled by dispatch).
  // -- This event should never get queued.
}

void HandleEvent_ProcessResource(emp::EventDrivenGP & hw, const event_t & event) {
  // Hardware doesn't need to do anything to handle this event (everything handled by dispatch).
  // -- This event should never get queued.
}

// Setup simple mutation function.
std::function<bool(Agent&, emp::Random&)> simple_mut_fun =
  [](Agent & agent, emp::Random & random) {
    program_t & program = agent.program;
    // Duplicate a function?
    if (random.P(fun_dup_rate) && program.GetSize() < MAX_FUNC_CNT) {
      const uint32_t fID = random.GetUInt(program.GetSize());
      program.PushFunction(program[fID]);
    }
    // Delete a function?
    if (random.P(fun_del_rate) && program.GetSize() > 1) {
      const uint32_t fID = random.GetUInt(program.GetSize());
      program[fID] = program[program.GetSize() - 1];
      program.program.resize(program.GetSize() - 1);
    }
    for (size_t fID = 0; fID < program.GetSize(); ++fID) {
      // Mutate affinity
      for (size_t i = 0; i < program[fID].affinity.GetSize(); ++i) {
        affinity_t & aff = program[fID].affinity;
        if (random.P(aff_bf_rate)) aff.Set(i, !aff.Get(i));
      }
      // Slip-mutation?
      if (random.P(slip_rate)) {
        uint32_t begin = random.GetUInt(program[fID].GetSize());
        uint32_t end = random.GetUInt(program[fID].GetSize());
        if (begin < end && ((program[fID].GetSize() + (end - begin)) < MAX_FUNC_LENGTH)) {
          // duplicate begin:end
          const size_t dup_size = end - begin;
          const size_t new_size = program[fID].GetSize() + dup_size;
          fun_t new_fun(program[fID].affinity);
          for (size_t i = 0; i < new_size; ++i) {
            if (i < end) new_fun.inst_seq.emplace_back(program[fID].inst_seq[i]);
            else new_fun.inst_seq.emplace_back(program[fID].inst_seq[i - dup_size]);
          }
          program[fID] = new_fun;
        } else if (begin > end && ((program[fID].GetSize() - (begin - end)) >= 1)) {
          // delete end:begin
          fun_t new_fun(program[fID].affinity);
          for (size_t i = 0; i < end; ++i)
            new_fun.inst_seq.emplace_back(program[fID].inst_seq[i]);
          for (size_t i = begin; i < program[fID].GetSize(); ++i)
            new_fun.inst_seq.emplace_back(program[fID].inst_seq[i]);
          program[fID] = new_fun;
        }
      }
      // Substitution mutations?
      for (size_t i = 0; i < program[fID].GetSize(); ++i) {
        inst_t & inst = program[fID].inst_seq[i];
        // Mutate affinity (even if it doesn't have one).
        for (size_t k = 0; k < inst.affinity.GetSize(); ++k) {
          if (random.P(aff_bf_rate)) inst.affinity.Set(k, !inst.affinity.Get(k));
        }
        // Mutate instruction.
        if (random.P(sub_rate)) inst.id = random.GetUInt(program.inst_lib->GetSize());
        // Mutate arguments (even if they aren't relevent to instruction).
        for (size_t k = 0; k < MAX_INST_ARGS; ++k) {
          if (random.P(sub_rate)) inst.args[k] = random.GetInt(MAX_ARG_VAL);
        }
      }
    }
    return (true);
  };

// Fitness function.
std::function<double(Agent&)> fit_fun =
  [](Agent & agent) {
    return (double)agent.resources_collected - (double)agent.poison_collected;
  };

int main() {

  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(RAND_SEED);
  emp::Ptr<inst_lib_t> inst_lib = emp::NewPtr<inst_lib_t>();
  emp::Ptr<event_lib_t> event_lib = emp::NewPtr<event_lib_t>();
  emp::World<Agent> world(random, "MegaSlush");

  // Environment variables.
  emp::vector<bool> env(EVAL_TIME, 0);
  size_t cur_time = 0;

  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  // Setup instruction library.
  inst_lib->AddInst("Inc", hardware_t::Inst_Inc, 1, "Increment value in local memory Arg1");
  inst_lib->AddInst("Dec", hardware_t::Inst_Dec, 1, "Decrement value in local memory Arg1");
  inst_lib->AddInst("Not", hardware_t::Inst_Not, 1, "Logically toggle value in local memory Arg1");
  inst_lib->AddInst("Add", hardware_t::Inst_Add, 3, "Local memory: Arg3 = Arg1 + Arg2");
  inst_lib->AddInst("Sub", hardware_t::Inst_Sub, 3, "Local memory: Arg3 = Arg1 - Arg2");
  inst_lib->AddInst("Mult", hardware_t::Inst_Mult, 3, "Local memory: Arg3 = Arg1 * Arg2");
  inst_lib->AddInst("Div", hardware_t::Inst_Div, 3, "Local memory: Arg3 = Arg1 / Arg2");
  inst_lib->AddInst("Mod", hardware_t::Inst_Mod, 3, "Local memory: Arg3 = Arg1 % Arg2");
  inst_lib->AddInst("TestEqu", hardware_t::Inst_TestEqu, 3, "Local memory: Arg3 = (Arg1 == Arg2)");
  inst_lib->AddInst("TestNEqu", hardware_t::Inst_TestNEqu, 3, "Local memory: Arg3 = (Arg1 != Arg2)");
  inst_lib->AddInst("TestLess", hardware_t::Inst_TestLess, 3, "Local memory: Arg3 = (Arg1 < Arg2)");
  inst_lib->AddInst("If", hardware_t::Inst_If, 1, "Local memory: If Arg1 != 0, proceed; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("While", hardware_t::Inst_While, 1, "Local memory: If Arg1 != 0, loop; else, skip block.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Countdown", hardware_t::Inst_Countdown, 1, "Local memory: Countdown Arg1 to zero.", emp::ScopeType::BASIC, 0, {"block_def"});
  inst_lib->AddInst("Close", hardware_t::Inst_Close, 0, "Close current block if there is a block to close.", emp::ScopeType::BASIC, 0, {"block_close"});
  inst_lib->AddInst("Break", hardware_t::Inst_Break, 0, "Break out of current block.");
  inst_lib->AddInst("Call", hardware_t::Inst_Call, 0, "Call function that best matches call affinity.", emp::ScopeType::BASIC, 0, {"affinity"});
  inst_lib->AddInst("Return", hardware_t::Inst_Return, 0, "Return from current function if possible.");
  inst_lib->AddInst("SetMem", hardware_t::Inst_SetMem, 2, "Local memory: Arg1 = numerical value of Arg2");
  inst_lib->AddInst("CopyMem", hardware_t::Inst_CopyMem, 2, "Local memory: Arg1 = Arg2");
  inst_lib->AddInst("SwapMem", hardware_t::Inst_SwapMem, 2, "Local memory: Swap values of Arg1 and Arg2.");
  inst_lib->AddInst("Input", hardware_t::Inst_Input, 2, "Input memory Arg1 => Local memory Arg2.");
  inst_lib->AddInst("Output", hardware_t::Inst_Output, 2, "Local memory Arg1 => Output memory Arg2.");
  inst_lib->AddInst("Commit", hardware_t::Inst_Commit, 2, "Local memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Pull", hardware_t::Inst_Pull, 2, "Shared memory Arg1 => Shared memory Arg2.");
  inst_lib->AddInst("Nop", hardware_t::Inst_Nop, 0, "No operation.");
  inst_lib->AddInst("SenseResource", Inst_SenseResource, 1);
  inst_lib->AddInst("ProcessResource", Inst_ProcessResource, 0);
  // Setup event library.
  event_lib->AddEvent("SenseResource", HandleEvent_SenseResource, "");
  event_lib->AddEvent("ProcessResource", HandleEvent_ProcessResource, "");

  event_lib->RegisterDispatchFun("SenseResource",
    [&cur_time, &env](hardware_t & hw_src, const event_t & event) {
      hw_src.SetTrait(TRAIT_ID__RES_SENSOR, (double)env.at(cur_time));
    });

  event_lib->RegisterDispatchFun("ProcessResource",
    [&cur_time, &env](hardware_t & hw_src, const event_t & event) {
      // If resources available, increment resources collected trait. Otherwise, increment poison collected trait.
      if (env.at(cur_time))
        hw_src.SetTrait(TRAIT_ID__RES_COLLECTED, hw_src.GetTrait(TRAIT_ID__RES_COLLECTED)+1.0);
      else
        hw_src.SetTrait(TRAIT_ID__POIS_COLLECTED, hw_src.GetTrait(TRAIT_ID__POIS_COLLECTED)+1.0);
    });

  // Generate ancestral seed program.
  program_t seed_program(inst_lib);
  seed_program.PushFunction(affinity_table[0]);
  for (size_t i = 0; i < MAX_FUNC_LENGTH / 2; ++i) seed_program.PushInst("Nop");
  Agent seed_agent(seed_program);

  // Configure the world.
  world.Inject(seed_agent, POP_SIZE);
  world.SetWellMixed(true);
  world.SetMutFun(simple_mut_fun);
  world.SetFitFun(fit_fun);

  // Configure evaluation hardware.
  hardware_t hardware(inst_lib, event_lib, random);
  hardware.SetMinBindThresh(MIN_BIND_THRESH);
  hardware.SetMaxCores(MAX_HW_CORES);

  // Do the run...
  double max_score = 0;
  double best_score = 0;
  for (size_t ud = 0; ud < GENERATIONS; ++ud) {
    // Generate an environment for this generation to experience.
    // env[0] = 1;
    env.at(0) = 1;  // Max score guaranteed to be at least 1.
    max_score = 0;
    for (size_t t = 1; t < EVAL_TIME; ++t) {
      ((emp::Mod((int)t, (int)ENV_CHG_CHECK) == 0) && (random->P(ENV_CHG_PERCENT))) ? (env.at(t) = !env.at(t-1)) : (env.at(t) = env.at(t-1));
      if (env.at(t)) ++max_score;
    }
    // Evaluate each agent.
    for (size_t id = 0; id < POP_SIZE; ++id) {
      // Configure hardware to run agent.
      hardware.ResetHardware();
      hardware.SetProgram(world[id].program);
      hardware.SetTrait(TRAIT_ID__RES_SENSOR, 0);
      hardware.SetTrait(TRAIT_ID__RES_COLLECTED, 0);
      hardware.SetTrait(TRAIT_ID__POIS_COLLECTED, 0);
      hardware.SpawnCore(0, memory_t(), true);
      // Run program for EVAL TIME.
      for (cur_time = 0; cur_time < EVAL_TIME; ++cur_time) {
        hardware.SingleProcess();
      }
      world.GetOrg(id).resources_collected = (size_t)hardware.GetTrait(TRAIT_ID__RES_COLLECTED);
      world.GetOrg(id).poison_collected = (size_t)hardware.GetTrait(TRAIT_ID__POIS_COLLECTED);
    }
    // Selection!
    // Keep the best agent.
    emp::EliteSelect(world, 1, 1);
    // Run a tournament for the rest.
    emp::TournamentSelect(world, 4, POP_SIZE-1);
    // Update the world (generational turnover)
    world.Update();
    // Mutate all but the first agent.
    world.DoMutations(1);
    // First agent is best of last generation, print fitness.
    best_score = fit_fun(world.GetOrg(0));
    std::cout << "Update # " << ud;
    std::cout << ", Max agent score: " << best_score << "(" << best_score/max_score << ")"<< std::endl;
  }
  // Run and print out best agent from last generation:
  std::cout << std::endl;
  std::cout << "Best program (score: "<< fit_fun(world.GetOrg(0)) << "):" << std::endl;
  std::cout << "--- Evaluating the best program. ---" << std::endl;
  hardware.ResetHardware();
  hardware.SetProgram(world[0].program);
  hardware.SetTrait(TRAIT_ID__RES_SENSOR, 0);
  hardware.SetTrait(TRAIT_ID__RES_COLLECTED, 0);
  hardware.SetTrait(TRAIT_ID__POIS_COLLECTED, 0);
  hardware.SpawnCore(0, memory_t(), true);
  for (cur_time = 0; cur_time < EVAL_TIME; ++cur_time) {
    hardware.SingleProcess();
  }
  hardware.PrintProgram(); std::cout << std::endl;
  hardware.PrintState(); std::cout << std::endl;

  return 0;
}
