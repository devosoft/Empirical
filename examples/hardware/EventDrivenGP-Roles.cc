
#include <iostream>
#include <unordered_set>
#include <utility>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"
#include "../../evo/World.h"

using event_t = typename emp::EventDrivenGP::event_t;
using event_lib_t = typename emp::EventDrivenGP::event_lib_t;
using affinity_t = emp::BitSet<8>;
using inst_t = typename::emp::EventDrivenGP::inst_t;
using inst_lib_t = typename::emp::EventDrivenGP::inst_lib_t;
using state_t = emp::EventDrivenGP::State;
using fun_t = emp::EventDrivenGP::Function;
using program_t = emp::EventDrivenGP::Program;

constexpr size_t POP_SIZE = 100;
constexpr size_t EVAL_TIME = 200;
constexpr size_t DIST_SYS_WIDTH = 5;
constexpr size_t DIST_SYS_HEIGHT = 5;
constexpr size_t DIST_SYS_SIZE = DIST_SYS_WIDTH * DIST_SYS_HEIGHT;
constexpr size_t GENERATIONS = 500;
constexpr int RAND_SEED = 2;

constexpr size_t TRAIT_ID__ROLE_ID = 1;
constexpr size_t TRAIT_ID__FITNESS = 0;
constexpr size_t TRAIT_ID__X_LOC = 2;
constexpr size_t TRAIT_ID__Y_LOC = 3;

constexpr size_t MAX_FUNC_LENGTH = 20;
constexpr size_t MAX_FUNC_CNT = 3;

// Mutation rates
//  * Affinity Mutations    -- per-bit flips
constexpr double aff_bf_rate = 0.05;
//  * Substitution          -- per-copy/site instruction substitutions
constexpr double sub_rate = 0.005;
//  * Insertion             -- per-copy/site instruction insertions
// constexpr double ins_rate = 0.005;
//  * Deletion              -- per-copy/site instruction deletions
// constexpr double del_rate = 0.005;
//  * Slip Mutations        -- per-function slip mutations
constexpr double slip_rate = 0.05;
//  * Function duplication  -- per-function whole-function duplications
constexpr double fun_dup_rate = 0.05;
//  * Function deletion     -- per-function whole-function deletion
constexpr double fun_del_rate = 0.05;

constexpr size_t CPU_SIZE = emp::EventDrivenGP::CPU_SIZE;
constexpr size_t MAX_INST_ARGS = emp::EventDrivenGP::MAX_INST_ARGS;

// This will be the target of evolution (what the world manages/etc.)
struct Agent {
  size_t valid_uid_cnt;
  size_t valid_id_cnt;
  program_t program;

  Agent(emp::Ptr<inst_lib_t> _ilib)
  : valid_uid_cnt(0), valid_id_cnt(0), program(_ilib) { ; }

  Agent(const program_t & _program)
  : valid_uid_cnt(0), valid_id_cnt(0), program(_program) { ; }

};

// Deme structure for holding distributed system.
struct Deme {
  using hardware_t = emp::EventDrivenGP;
  using memory_t = typename emp::EventDrivenGP::memory_t;
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

  Deme(emp::Ptr<emp::Random> _rnd, size_t _w, size_t _h, emp::Ptr<event_lib_t> _elib, emp::Ptr<inst_lib_t> _ilib)
    : grid(_w * _h), width(_w), height(_h), rnd(_rnd), event_lib(_elib), inst_lib(_ilib), agent_ptr(nullptr), agent_loaded(false) {
    // Register dispatch function.
    event_lib->RegisterDispatchFun("Message", [this](hardware_t & hw_src, const event_t & event){ this->DispatchMessage(hw_src, event); });
    // Fill out the grid with hardware.
    for (size_t i = 0; i < width * height; ++i) {
      grid[i].New(inst_lib, event_lib, rnd);
      pos_t pos = GetPos(i);
      grid[i]->SetTrait(TRAIT_ID__ROLE_ID, 0);
      grid[i]->SetTrait(TRAIT_ID__FITNESS, 0);
      grid[i]->SetTrait(TRAIT_ID__X_LOC, pos.first);
      grid[i]->SetTrait(TRAIT_ID__Y_LOC, pos.second);
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
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->ResetHardware();
      grid[i]->SetTrait(TRAIT_ID__ROLE_ID, 0);
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
  }

  size_t GetWidth() const { return width; }
  size_t GetHeight() const { return height; }

  pos_t GetPos(size_t id) { return pos_t(id % width, id / width); }
  size_t GetID(size_t x, size_t y) { return (y * width) + x; }

  void Print(std::ostream & os=std::cout) {
    os << "=============DEME=============\n";
    for (size_t i = 0; i < grid.size(); ++i) {
      os << "--- Agent @ (" << GetPos(i).first << ", " << GetPos(i).second << ") ---\n";
      grid[i]->PrintState(os); os << "\n";
    }
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

  // Pulled this function from PopMng_Grid.
  size_t GetRandomNeighbor(size_t id) {
    const int offset = rnd->GetInt(9);
    const int rand_x = (int)(id%width) + offset%3-1;
    const int rand_y = (int)(id/width) + offset/3-1;
    return ((size_t)emp::Mod(rand_x, (int)width) + (size_t)emp::Mod(rand_y, (int)height)*width);
  }


  void Advance(size_t t=1) { for (size_t i = 0; i < t; ++i) SingleAdvance(); }

  void SingleAdvance() {
    emp_assert(agent_loaded);
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->SingleProcess();
    }
  }

};

// Some extra instructions for this experiment.
void Inst_GetRoleID(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = *hw.GetCurState();
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__ROLE_ID));
}

void Inst_SetRoleID(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = *hw.GetCurState();
  hw.SetTrait(TRAIT_ID__ROLE_ID, (int)state.AccessLocal(inst.args[0]));
}

void Inst_GetXLoc(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = *hw.GetCurState();
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__X_LOC));
}

void Inst_GetYLoc(emp::EventDrivenGP & hw, const inst_t & inst) {
  state_t & state = *hw.GetCurState();
  state.SetLocal(inst.args[0], hw.GetTrait(TRAIT_ID__Y_LOC));

}

int main() {

  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  // Setup random number generator.
  emp::Ptr<emp::Random> random;
  random.New(RAND_SEED);

  // Setup the instruction library for this experiment.
  emp::Ptr<inst_lib_t> inst_lib = emp::NewPtr<inst_lib_t>(*emp::EventDrivenGP::DefaultInstLib());
  emp::Ptr<event_lib_t> event_lib = emp::NewPtr<event_lib_t>(*emp::EventDrivenGP::DefaultEventLib());

  inst_lib->AddInst("GetRoleID", Inst_GetRoleID, 1, "Local memory[Arg1] = Trait[RoleID]");
  inst_lib->AddInst("SetRoleID", Inst_SetRoleID, 1, "Trait[RoleID] = Local memory[Arg1]");
  inst_lib->AddInst("GetXLoc", Inst_GetXLoc, 1, "Local memory[Arg1] = Trait[XLoc]");
  inst_lib->AddInst("GetYLoc", Inst_GetYLoc, 1, "Local memory[Arg1] = Trait[YLoc]");

  // // Configure a seed program.
  program_t seed_program(inst_lib);
  seed_program.PushFunction(fun_t(affinity_table[1]));
  for (size_t i = 0; i < MAX_FUNC_LENGTH; i++) seed_program.PushInst("Nop");
  // seed_program.PushInst("Inc", 0);
  // seed_program.PushInst("SetRoleID", 0);  // Set roleID to 1
  // seed_program.PushInst("BroadcastMsg", 0, 0, 0, affinity_table[0]);
  // seed_program.PushFunction(fun_t(affinity_table[255]));
  // for (size_t i = 0; i < 3; ++i) seed_program.PushInst(i);
  // for (size_t i = 0; i < 5; i++) seed_program.PushInst("Nop");

  Agent seed_agent(seed_program);

  Deme eval_deme(random, DIST_SYS_WIDTH, DIST_SYS_HEIGHT, event_lib, inst_lib);

  emp::evo::EAWorld<Agent> world(random, "Distributed-Role-World");
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


  // Setup unique role ID fitness function. From: (Goldsby et al. 2010).
  std::function<double(Agent*)> fit_fun =
    [](Agent * agent) {
      return (agent->valid_id_cnt >= DIST_SYS_SIZE) ? (agent->valid_id_cnt + agent->valid_uid_cnt) : (agent->valid_id_cnt);
    };

  // Agent * agent = new Agent(seed_program);
  // std::cout << "--- Before mutations: ---" << std::endl;
  // agent->program.PrintProgram(); std::cout << std::endl;
  // simple_mut_fun(agent, *random);
  // std::cout << "--- After mutations: ---" << std::endl;
  // agent->program.PrintProgram(); std::cout << std::endl;

  world.SetDefaultMutateFun(simple_mut_fun);

  // Do the run...
  for (size_t ud = 0; ud < GENERATIONS; ++ud) {
    std::cout << "Update #" << ud << std::endl;
    // Evaluate each agent.
    for (size_t id = 0; id < POP_SIZE; ++id) {
      eval_deme.LoadAgent(world.popM[id]);
      eval_deme.Advance(EVAL_TIME);
      std::unordered_set<double> valid_uids;
      size_t valid_id_cnt = 0;
      for (size_t i = 0; i < eval_deme.grid.size(); ++i) {
        const double role_id = eval_deme.grid[i]->GetTrait(TRAIT_ID__ROLE_ID);
        if (role_id > 0 && role_id <= DIST_SYS_SIZE) {
          ++valid_id_cnt; // Increment valid id cnt.
          valid_uids.insert(role_id); // Add to set.
        }
      }
      world[id].valid_uid_cnt = valid_uids.size();
      world[id].valid_id_cnt = valid_id_cnt;
    }
    // Keep the best agent.
    world.EliteSelect(fit_fun, 1, 1);
    // Run a tournament for the rest.
    world.TournamentSelect(fit_fun, 8, POP_SIZE - 1);
    // Update the world (generational turnover)
    world.Update();
    // Mutate all but the first agent.
    world.MutatePop(1);
    // First agent is the best of the last generation.
    std::cout << "  Max score: " << fit_fun(world.popM[0]) << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Best program (valid ids: " << world[0].valid_id_cnt << ", unique valid ids: "<< world[0].valid_uid_cnt <<"): " << std::endl;
  world[0].program.PrintProgram(); std::cout << std::endl;
  std::cout << "--- Evaluating best program. ---" << std::endl;
  eval_deme.LoadAgent(world.popM[0]);
  eval_deme.Advance(EVAL_TIME);
  eval_deme.Print(); std::cout << std::endl;

  inst_lib.Delete();
  event_lib.Delete();
  random.Delete();
  return 0;
}
