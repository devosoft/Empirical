
// Evolving distributed algorithms to solve a distributed energy conservation problem.
// Environment: Systems (demes) of agents must accumulate resources by processing a randomly available
//  environmental resource. Individual agents each have finite energy reserves that are used to execute
//  instructions (1 SinleProcess costs X energy). Energy reserves are not sufficient for individuals to
//  remain in an active state for the entirety of a deme evaluation. Agents may enter a sleep state to
//  preserve energy.

#include <iostream>

#include "base/vector.h"
#include "hardware/EventDrivenGP.h"
#include "base/Ptr.h"
#include "tools/Random.h"
#include "tools/random_utils.h"
#include "tools/BitSet.h"
#include "evo/World.h"


using event_t = typename emp::EventDrivenGP::event_t;
using event_lib_t = typename emp::EventDrivenGP::event_lib_t;
using affinity_t = emp::BitSet<8>;
using inst_t = typename::emp::EventDrivenGP::inst_t;
using inst_lib_t = typename::emp::EventDrivenGP::inst_lib_t;
using state_t = emp::EventDrivenGP::State;
using fun_t = emp::EventDrivenGP::Function;
using program_t = emp::EventDrivenGP::Program;
using hardware_t = emp::EventDrivenGP;
using memory_t = typename emp::EventDrivenGP::memory_t;

constexpr size_t POP_SIZE = 300;
constexpr size_t EVAL_TIME = 400;
constexpr size_t DIST_SYS_WIDTH = 5;
constexpr size_t DIST_SYS_HEIGHT = 5;
constexpr size_t GENERATIONS = 1000;
constexpr int RAND_SEED = -1;

constexpr double RESOURCE_AVAILABILITY = 0.3;
constexpr size_t RESOURCE_NUM_TIME_CHUNKS = 40;

constexpr size_t SLEEP_TIME = 10; // How many CPU cycles should sleep last?
constexpr size_t INIT_ENERGY = 50;

constexpr size_t MAX_FUNC_LENGTH = 20;
constexpr size_t MAX_FUNC_CNT = 3;

// Virtual hardware trait IDs.
constexpr size_t TRAIT_ID__X_LOC = 0;
constexpr size_t TRAIT_ID__Y_LOC = 1;
constexpr size_t TRAIT_ID__ENERGY = 2;
constexpr size_t TRAIT_ID__SLEEP_CNT = 3;
constexpr size_t TRAIT_ID__RES_SENSOR = 4;
constexpr size_t TRAIT_ID__RES_COLLECTED = 5;
constexpr size_t TRAIT_ID__POIS_COLLECTED = 6;
constexpr size_t TRAIT_ID__PROCESSED = 7;


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

constexpr size_t CPU_SIZE = emp::EventDrivenGP::CPU_SIZE;
constexpr size_t MAX_INST_ARGS = emp::EventDrivenGP::MAX_INST_ARGS;

struct Agent {
  program_t program;
  size_t resources_collected;
  size_t poison_collected;
  size_t time_survived;

  Agent(const program_t & _p)
    : program(_p), resources_collected(0), poison_collected(0), time_survived(0) { ; }
};

struct Environment {
  emp::Ptr<emp::Random> rnd;
  size_t num_chunks;
  double rate_available;
  size_t max_time;
  size_t chunk_len;
  size_t chunks_avail;

  emp::vector<bool> res_availability;

  Environment(emp::Ptr<emp::Random> _rnd, size_t _nchunks, size_t _mtime, double _ravail)
  : rnd(_rnd), num_chunks(_nchunks), rate_available(_ravail), max_time(_mtime), chunk_len(_mtime / _nchunks),
    chunks_avail((size_t)(_ravail * _nchunks)), res_availability(_nchunks, false)
  {
    for (size_t i = 0; i < chunks_avail; ++i) res_availability[i] = true;
    RandomizeEnv();
  }

  void RandomizeEnv() {
    emp::Shuffle(*rnd, res_availability);
  }

  /// Get Environment state at time t.
  bool GetEnvState(size_t t) {
    emp_assert(t <= max_time);
    size_t in_chunk = t / chunk_len;
    return res_availability[in_chunk];
  }
};

/// Struct to represent homogeneous deme (with torodial grid topology).
struct Deme {
  using grid_t = emp::vector<emp::Ptr<hardware_t>>;
  using pos_t = std::pair<size_t, size_t>;

  grid_t grid;
  size_t width;
  size_t height;
  emp::Ptr<emp::Random> rnd;
  emp::Ptr<event_lib_t> event_lib;
  emp::Ptr<inst_lib_t> inst_lib;

  emp::Ptr<Agent> agent_ptr;
  bool agent_loaded;
  bool deme_active;

  Deme(emp::Ptr<emp::Random> _rnd, size_t _w, size_t _h, emp::Ptr<event_lib_t> _elib, emp::Ptr<inst_lib_t> _ilib)
    : grid(_w * _h), width(_w), height(_h), rnd(_rnd), event_lib(_elib), inst_lib(_ilib), agent_ptr(nullptr),
      agent_loaded(false), deme_active(false) {
    // Register dispatch function(s) - Message, Wake.
    event_lib->RegisterDispatchFun("Message", [this](hardware_t & hw_src, const event_t & event){ this->DispatchMessage(hw_src, event); });
    event_lib->RegisterDispatchFun("Wake", [this](hardware_t & hw_src, const event_t & event) { this->DispatchWake(hw_src, event); });
    // Fill out the grid with hardware.
    for (size_t i = 0; i < width * height; ++i) {
      grid[i].New(inst_lib, event_lib, rnd);
      pos_t pos = GetPos(i);
      grid[i]->SetTrait(TRAIT_ID__X_LOC, pos.first);
      grid[i]->SetTrait(TRAIT_ID__Y_LOC, pos.second);
      grid[i]->SetTrait(TRAIT_ID__ENERGY, 0);
      grid[i]->SetTrait(TRAIT_ID__SLEEP_CNT, 0);
      grid[i]->SetTrait(TRAIT_ID__RES_SENSOR, 0);
      grid[i]->SetTrait(TRAIT_ID__RES_COLLECTED, 0);
      grid[i]->SetTrait(TRAIT_ID__POIS_COLLECTED, 0);
      grid[i]->SetTrait(TRAIT_ID__PROCESSED, 0);
    }
  }

  ~Deme() {
    Reset();
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i].Delete();
    }
    grid.resize(0);
  }

  void Reset() {
    agent_ptr = nullptr;
    agent_loaded = false;
    deme_active = false;
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->ResetHardware();
      grid[i]->SetTrait(TRAIT_ID__ENERGY, INIT_ENERGY);
      grid[i]->SetTrait(TRAIT_ID__SLEEP_CNT, 0);
      grid[i]->SetTrait(TRAIT_ID__RES_SENSOR, 0);
      grid[i]->SetTrait(TRAIT_ID__RES_COLLECTED, 0);
      grid[i]->SetTrait(TRAIT_ID__POIS_COLLECTED, 0);
      grid[i]->SetTrait(TRAIT_ID__PROCESSED, 0);
    }
  }

  void LoadAgent(emp::Ptr<Agent> _agent_ptr) {
    Reset();
    agent_ptr = _agent_ptr;
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->SetProgram(agent_ptr->program);
      grid[i]->SpawnCore(0, memory_t(), true);
    }
    agent_loaded = true;
    deme_active = true;
  }

  size_t GetWidth() const { return width; }
  size_t GetHeight() const { return height; }

  bool IsActive() const { return deme_active; }

  pos_t GetPos(size_t id) { return pos_t(id % width, id / width); }
  size_t GetID(size_t x, size_t y) { return (y * width) + x; }

  double GetDemeResources() {
    emp_assert(agent_loaded);
    double cnt = 0;
    for (size_t i = 0; i < grid.size(); ++i) {
      cnt += grid[i]->GetTrait(TRAIT_ID__RES_COLLECTED);
    }
    return cnt;
  }

  double GetDemePoison() {
    emp_assert(agent_loaded);
    double cnt = 0;
    for (size_t i = 0; i < grid.size(); ++i) {
      cnt += grid[i]->GetTrait(TRAIT_ID__POIS_COLLECTED);
    }
    return cnt;
  }

  // Pulled this function from PopMng_Grid.
  size_t GetRandomNeighbor(size_t id) {
    const int offset = rnd->GetInt(9);
    const int rand_x = (int)(id%width) + offset%3-1;
    const int rand_y = (int)(id/width) + offset/3-1;
    return ((size_t)emp::Mod(rand_x, (int)width) + (size_t)emp::Mod(rand_y, (int)height)*width);
  }

  void DispatchMessage(hardware_t & hw_src, const event_t & event) {
    const size_t x = (size_t)hw_src.GetTrait(TRAIT_ID__X_LOC);
    const size_t y = (size_t)hw_src.GetTrait(TRAIT_ID__Y_LOC);
    emp::vector<size_t> recipients;
    if (event.HasProperty("send")) {
      // Send to random neighbor.
      recipients.push_back(GetRandomNeighbor(GetID(x, y)));
    } else {
      // Treat as broadcast, send to all neighbors.
      recipients.push_back(GetID((size_t)emp::Mod((int)x - 1, (int)width), (size_t)emp::Mod((int)y, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x + 1, (int)width), (size_t)emp::Mod((int)y, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x, (int)width), (size_t)emp::Mod((int)y - 1, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x, (int)width), (size_t)emp::Mod((int)y + 1, (int)height)));
    }
    // Dispatch event to recipients.
    for (size_t i = 0; i < recipients.size(); ++i)
      grid[recipients[i]]->QueueEvent(event);
  }

  void DispatchWake(hardware_t & hw_src, const event_t & event) {
    const size_t x = (size_t)hw_src.GetTrait(TRAIT_ID__X_LOC);
    const size_t y = (size_t)hw_src.GetTrait(TRAIT_ID__Y_LOC);
    emp::vector<size_t> recipients;
    if (event.HasProperty("send")) {
      // Send to random neighbor.
      recipients.push_back(GetRandomNeighbor(GetID(x, y)));
    } else {
      // Treat as broadcast, send to all neighbors.
      recipients.push_back(GetID((size_t)emp::Mod((int)x - 1, (int)width), (size_t)emp::Mod((int)y, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x + 1, (int)width), (size_t)emp::Mod((int)y, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x, (int)width), (size_t)emp::Mod((int)y - 1, (int)height)));
      recipients.push_back(GetID((size_t)emp::Mod((int)x, (int)width), (size_t)emp::Mod((int)y + 1, (int)height)));
    }
    // Wake up recipients.
    for (size_t i = 0; i < recipients.size(); ++i)
      grid[recipients[i]]->SetTrait(TRAIT_ID__SLEEP_CNT, 0);
  }

  void Advance(size_t t=1) { for (size_t i = 0; i < t; ++i) SingleAdvance(); }

  void SingleAdvance() {
    emp_assert(agent_loaded);
    // For each agent: (Naive scheduler -- just loop through agents in order)
    deme_active = false;
    for (size_t i = 0; i < grid.size(); ++i) {
      // Make resources available to this agent. (just 1)
      grid[i]->SetTrait(TRAIT_ID__PROCESSED, 0);
      // Enough energy to execute and not asleep?
      const double energy = grid[i]->GetTrait(TRAIT_ID__ENERGY);
      const int sleep_cnt = (int)grid[i]->GetTrait(TRAIT_ID__SLEEP_CNT);
      if (energy > 0 && !sleep_cnt) {
        // If enough energy and not sleeping, impose cost and execute.
        grid[i]->SetTrait(TRAIT_ID__ENERGY, energy - 1);
        grid[i]->SingleProcess();
        deme_active = true;
      } else if (energy > 0) {
        // If enough energy, but sleeping, dec sleep counter.
        grid[i]->SetTrait(TRAIT_ID__SLEEP_CNT, sleep_cnt - 1);
        deme_active = true;
      }
    }
  }

  void Print(std::ostream & os=std::cout) {
    os << "=============DEME=============\n";
    for (size_t i = 0; i < grid.size(); ++i) {
      os << "--- Agent @ (" << GetPos(i).first << ", " << GetPos(i).second << ") ---\n";
      grid[i]->PrintState(os); os << "\n";
    }
  }
};

// Extra instructions for this experiment.
void Inst_Sleep(emp::EventDrivenGP & hw, const inst_t & inst) {
  hw.SetTrait(TRAIT_ID__SLEEP_CNT, SLEEP_TIME);
}

void Inst_SendWake(emp::EventDrivenGP & hw, const inst_t & inst) {
  hw.TriggerEvent("Wake", affinity_t(), {}, {"send"});
}

void Inst_BroadcastWake(emp::EventDrivenGP & hw, const inst_t & inst) {
  hw.TriggerEvent("Wake", affinity_t(), {}, {"broadcast"});
}

void Inst_SenseResource(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  hw.TriggerEvent("SenseResource"); // Event dispatch should do the sensing.
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__RES_SENSOR));
}

void Inst_ProcessResource(emp::EventDrivenGP & hw, const inst_t & inst) {
  hw.TriggerEvent("ProcessResource");
}

void Inst_GetXLoc(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__X_LOC));
}

void Inst_GetYLoc(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = hw.GetCurState();
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__Y_LOC));
}

void HandleEvent_Wake(emp::EventDrivenGP & hw, const event_t & event) {
  // Hardware doesn't need to do anything to handle this event (everything handled by dispatch).
}

void HandleEvent_SenseResource(emp::EventDrivenGP & hw, const event_t & event) {
  // Hardware doesn't need to do anything to handle this event (everything handled by dispatch).
}

void HandleEvent_ProcessResource(emp::EventDrivenGP & hw, const event_t & event) {
  // Hardware doesn't need to do anything to handle this event (everything handled by dispatch).
}

int main() {
  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  emp::Ptr<emp::Random> random = emp::NewPtr<emp::Random>(RAND_SEED);

  // Test environment.
  Environment env(random, RESOURCE_NUM_TIME_CHUNKS, EVAL_TIME, RESOURCE_AVAILABILITY);
  size_t cur_time = 0;

  // Configure the instruction library and the event library.
  emp::Ptr<inst_lib_t> inst_lib = emp::NewPtr<inst_lib_t>(*emp::EventDrivenGP::DefaultInstLib());
  emp::Ptr<event_lib_t> event_lib = emp::NewPtr<event_lib_t>(*emp::EventDrivenGP::DefaultEventLib());

  inst_lib->AddInst("Sleep", Inst_Sleep, 0);
  inst_lib->AddInst("SendWake", Inst_SendWake, 0);
  inst_lib->AddInst("BroadcastWake", Inst_BroadcastWake, 0);
  inst_lib->AddInst("SenseResource", Inst_SenseResource, 1);
  inst_lib->AddInst("ProcessResource", Inst_ProcessResource, 0);
  inst_lib->AddInst("GetXLoc", Inst_GetXLoc, 1, "Local memory[Arg1] = Trait[XLoc]");
  inst_lib->AddInst("GetYLoc", Inst_GetYLoc, 1, "Local memory[Arg1] = Trait[YLoc]");

  event_lib->AddEvent("Wake", HandleEvent_Wake, "Event for waking agents from a sleep state.");
  event_lib->AddEvent("SenseResource", HandleEvent_SenseResource, "");
  event_lib->AddEvent("ProcessResource", HandleEvent_ProcessResource, "");

  event_lib->RegisterDispatchFun("SenseResource",
    [&cur_time, &env](hardware_t & hw_src, const event_t & event) {
      hw_src.SetTrait(TRAIT_ID__RES_SENSOR, (double)env.GetEnvState(cur_time));
    });

  event_lib->RegisterDispatchFun("ProcessResource",
    [&cur_time, &env](hardware_t & hw_src, const event_t & event) {
      if (hw_src.GetTrait(TRAIT_ID__PROCESSED) == 0) {
        hw_src.SetTrait(TRAIT_ID__PROCESSED, 1);
        if (env.GetEnvState(cur_time))
          hw_src.SetTrait(TRAIT_ID__RES_COLLECTED, hw_src.GetTrait(TRAIT_ID__RES_COLLECTED)+1.0);
        else
          hw_src.SetTrait(TRAIT_ID__POIS_COLLECTED, hw_src.GetTrait(TRAIT_ID__POIS_COLLECTED)+1.0);
      }
    });

  std::cout << "Environment: " << std::endl;
  for (size_t i = 0; i < env.res_availability.size(); ++i)
    std::cout << env.res_availability[i] << " ";
  std::cout << std::endl;

  program_t seed_program(inst_lib);
  seed_program.PushFunction(fun_t(affinity_table[0]));
  for (size_t i = 0; i < MAX_FUNC_LENGTH; ++i) seed_program.PushInst("Nop");
  // seed_program.PushInst("Nop");
  // seed_program.PushInst("GetXLoc", 0);
  // seed_program.PushInst("GetYLoc", 1);
  // seed_program.PushInst("Add", 0, 1, 2);
  // seed_program.PushInst("If", 2);
  // seed_program.PushInst("Sleep");
  // seed_program.PushInst("Close");
  // seed_program.PushInst("Nop");
  // seed_program.PushInst("Nop");
  // seed_program.PushInst("BroadcastWake");
  // seed_program.PushInst("Inc", 4);
  // seed_program.PushInst("While", 4);

  Agent seed_agent(seed_program);
  Deme eval_deme(random, DIST_SYS_WIDTH, DIST_SYS_HEIGHT, event_lib, inst_lib);

  // Test demes.
  // eval_deme.LoadAgent(&seed_agent);
  // std::cout << "-- Initial state --" << std::endl;
  // eval_deme.Print();
  // for (cur_time = 0; cur_time < EVAL_TIME && eval_deme.IsActive(); ++cur_time) {
  //   std::cout << "-- Time: " << cur_time << std::endl;
  //   eval_deme.SingleAdvance();
  //   eval_deme.Print();
  // }

  emp::evo::EAWorld<Agent> world(random, "Sleepy-World");
  world.Insert(seed_agent, POP_SIZE);

  // Setup simple mutation function.
  std::function<bool(Agent*, emp::Random&)> simple_mut_fun =
    [](Agent *agent, emp::Random& random) {
      program_t & program = agent->program;
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
            if (random.P(sub_rate)) inst.args[k] = random.GetUInt(CPU_SIZE);
          }
        }
      }
      return (true);
    };

  // Setup fitness function(s).
  std::function<double(Agent*)> fit_fun__just_res =
    [](Agent * agent) {
      return (double)agent->resources_collected;
    };
  std::function<double(Agent*)> fit_fun__res_pois =
    [](Agent * agent) {
      return (double)agent->resources_collected - (double)agent->poison_collected;
    };
  std::function<double(Agent*)> fit_fun__time_survived =
   [](Agent * agent) {
     return (double)agent->time_survived;
   };
   std::function<double(Agent*)> fit_fun__res_pois2 =
    [](Agent * agent) {
      return (2*(double)agent->resources_collected) - (double)agent->poison_collected;
    };
   emp::vector< std::function<double(Agent*)> > fit_set(3);
   fit_set[0] = fit_fun__just_res;
   fit_set[1] = fit_fun__res_pois;
   fit_set[2] = fit_fun__time_survived;


  world.SetDefaultMutateFun(simple_mut_fun);

  // Do the run...
  for (size_t ud = 0; ud < GENERATIONS; ++ud) {
    std::cout << "Update #" << ud << std::endl;
    env.RandomizeEnv();
    for (size_t id = 0; id < POP_SIZE; ++id) {
      eval_deme.LoadAgent(world.popM[id]);
      for (cur_time = 0; cur_time < EVAL_TIME && eval_deme.IsActive(); ++cur_time) {
        eval_deme.SingleAdvance();
      }
      // How long did the deme last?
      world[id].time_survived = cur_time;
      world[id].resources_collected = eval_deme.GetDemeResources();
      world[id].poison_collected = eval_deme.GetDemePoison();
    }
    // Keep the best agent.
    world.EliteSelect(fit_fun__res_pois, 1, 1);
    // Run a tournament for the rest.
    world.TournamentSelect(fit_fun__res_pois2, 8, POP_SIZE - 1);
    //world.LexicaseSelect(fit_set, POP_SIZE - 1);
    // Update the world (generational turnover).
    world.Update();
    // Mutate all but the first agent.
    world.MutatePop(1);
    // First agent is best of last generation, print fitness.
    // std::cout << "  RP score: " << fit_fun__res_pois(world.popM[0]) << std::endl;
    std::cout << "  RP2 score: " << fit_fun__res_pois2(world.popM[0]) << std::endl;
    std::cout << "  R score: " << world[0].resources_collected << std::endl;
    std::cout << "  P score: " << world[0].poison_collected << std::endl;
    std::cout << "  T score: " << world[0].time_survived << std::endl;
  }


  std::cout << std::endl;
  std::cout << "Best program" << std::endl;
  std::cout << "  RP2 score: " << fit_fun__res_pois2(world.popM[0]) << std::endl;
  std::cout << "  R score: " << world[0].resources_collected << std::endl;
  std::cout << "  P score: " << world[0].poison_collected << std::endl;
  std::cout << "  T score: " << world[0].time_survived << std::endl;
  world[0].program.PrintProgram(); std::cout << std::endl;
  std::cout << "--- Evaluating best program. ---" << std::endl;
  std::cout << "Environment: " << std::endl;
  for (size_t i = 0; i < env.res_availability.size(); ++i)
    std::cout << env.res_availability[i] << " ";
  std::cout << std::endl;
  eval_deme.LoadAgent(world.popM[0]);
  for (cur_time = 0; cur_time < EVAL_TIME && eval_deme.IsActive(); ++cur_time) {
    eval_deme.SingleAdvance();
  }
  world[0].time_survived = cur_time;
  world[0].resources_collected = eval_deme.GetDemeResources();
  world[0].poison_collected = eval_deme.GetDemePoison();
  eval_deme.Print(); std::cout << std::endl;
  return 0;
}
