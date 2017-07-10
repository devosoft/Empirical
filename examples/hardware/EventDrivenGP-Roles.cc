
#include <iostream>
#include <utility>
#include "../../hardware/EventDrivenGP.h"
#include "../../base/Ptr.h"
#include "../../tools/Random.h"
#include "../../tools/BitSet.h"
#include "../../evo/World.h"

// Things to do:
//  [ ] Define fitness function
//    [ ] Build a 'deme' and evaluate it.
//  [ ] Define mutation function.  // Making a table of bit set values to make affinity assignment easier.

using event_t = typename emp::EventDrivenGP::event_t;
using event_lib_t = typename emp::EventDrivenGP::event_lib_t;
using affinity_t = emp::BitSet<8>;
using inst_t = typename::emp::EventDrivenGP::inst_t;
using inst_lib_t = typename::emp::EventDrivenGP::inst_lib_t;
using state_t = emp::EventDrivenGP::State;
using fun_t = emp::EventDrivenGP::Function;
using program_t = emp::EventDrivenGP::Program;

constexpr size_t POP_SIZE = 10;
constexpr size_t EVAL_TIME = 55;
constexpr size_t DIST_SYS_WIDTH = 3;
constexpr size_t DIST_SYS_HEIGHT = 3;

constexpr size_t TRAIT_ID__ROLE_ID = 1;
constexpr size_t TRAIT_ID__FITNESS = 0;
constexpr size_t TRAIT_ID__X_LOC = 2;
constexpr size_t TRAIT_ID__Y_LOC = 3;


// Deme structure for holding distributed system.
struct Deme {
  using agent_t = emp::EventDrivenGP;
  using memory_t = typename emp::EventDrivenGP::memory_t;
  using grid_t = emp::vector<emp::Ptr<agent_t>>;
  using pos_t = std::pair<size_t, size_t>;

  grid_t grid;
  size_t width;
  size_t height;
  emp::Ptr<emp::Random> rnd;
  emp::Ptr<event_lib_t> event_lib;
  emp::Ptr<inst_lib_t> inst_lib;

  program_t program;
  bool loaded_program;

  Deme(emp::Ptr<emp::Random> _rnd, size_t _w, size_t _h, emp::Ptr<event_lib_t> _elib, emp::Ptr<inst_lib_t> _ilib)
    : grid(_w * _h), width(_w), height(_h), rnd(_rnd), event_lib(_elib), inst_lib(_ilib), program(_ilib), loaded_program(false) {
    // Register dispatch function.
    event_lib->RegisterDispatchFun("Message", [this](agent_t & hw_src, const event_t & event){ this->DispatchMessage(hw_src, event); });
    // Fill out the grid with hardware.
    for (size_t i = 0; i < width * height; ++i) {
      grid[i].New(inst_lib, event_lib, rnd);
      pos_t pos = GetPos(i);
      grid[i]->SetTrait(TRAIT_ID__ROLE_ID, 0);
      grid[i]->SetTrait(TRAIT_ID__FITNESS, 0);
      grid[i]->SetTrait(TRAIT_ID__X_LOC, pos.first);
      grid[i]->SetTrait(TRAIT_ID__Y_LOC, pos.second);
      grid[i]->PrintTraits(); std::cout << std::endl;
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
    loaded_program = false;
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->ResetHardware();
    }
  }

  void LoadProgram(const program_t & _program) {
    Reset();
    for (size_t i = 0; i < grid.size(); ++i) {
      grid[i]->SetProgram(_program);
      grid[i]->SpawnCore(0, memory_t(), true);
    }
    loaded_program = true;
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

  void DispatchMessage(agent_t & hw_src, const event_t & event) {
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


  void SingleAdvance() {
    emp_assert(loaded_program);
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

int main() {
  std::cout << "Testing EventDrivenGP." << std::endl;

  // Some convenient type aliases:

  // Define a convenient affinity table.
  emp::vector<affinity_t> affinity_table(256);
  for (size_t i = 0; i < affinity_table.size(); ++i) {
    affinity_table[i].SetByte(0, (uint8_t)i);
  }

  // Setup random number generator.
  emp::Ptr<emp::Random> random;
  random.New(2);

  // Setup the instruction library for this experiment.
  emp::Ptr<inst_lib_t> inst_lib;
  inst_lib.New(*emp::EventDrivenGP::DefaultInstLib());

  emp::Ptr<inst_lib_t> inst_lib2;
  inst_lib2.New(*emp::EventDrivenGP::DefaultInstLib());

  emp::Ptr<event_lib_t> event_lib;
  event_lib.New(*emp::EventDrivenGP::DefaultEventLib());


  inst_lib->AddInst("GetRoleID", Inst_GetRoleID, 1, "Local memory[Arg1] = Trait[RoleID]");
  inst_lib->AddInst("SetRoleID", Inst_SetRoleID, 1, "Trait[RoleID] = Local memory[Arg1]");

  std::cout << "Inst lib1: " << std::endl;
  for (size_t i = 0; i < inst_lib->GetSize(); ++i) {
    std::cout << i << ": " << inst_lib->GetName(i) << std::endl;
  }
  std::cout << "Inst lib2: " << std::endl;
  for (size_t i = 0; i < inst_lib2->GetSize(); ++i) {
    std::cout << i << ": " << inst_lib2->GetName(i) << std::endl;
  }

  // Dummy dispatch function for testing.
  // event_lib->RegisterDispatchFun("Message",
  //                               [](emp::EventDrivenGP &, const event_t &) {
  //                                 std::cout << "Dispatch plz?" << std::endl;
  //                               });
  //
  // // Configure a seed program.
  // program_t seed_program(inst_lib);
  //
  // seed_program.PushFunction(fun_t(affinity_table[0]));
  // for (size_t i = 0; i < 48; i++) seed_program.PushInst("Nop");
  // seed_program.PushInst("Inc", 0);
  // seed_program.PushInst("SetRoleID", 0);  // Set roleID to 1
  // seed_program.PushInst("BroadcastMsg", 0, 0, 0, affinity_table[0]);
  //
  // Deme eval_deme(random, DIST_SYS_WIDTH, DIST_SYS_HEIGHT, event_lib, inst_lib);
  // eval_deme.LoadProgram(seed_program);
  // // Testing deme.
  // eval_deme.Print();
  // for (size_t i = 0; i < 5; ++i) {
  //   std::cout << "||||||||||||||||DEME UPDATE(" << i << ")||||||||||||||||" << std::endl;
  //   eval_deme.SingleAdvance();
  //   eval_deme.Print();
  // }
  //
  // // @amlalejini: Wishing evo3 worked...
  // emp::evo::EAWorld<program_t> world(random, "Distributed-Role-World");
  // world.Insert(seed_program, POP_SIZE);



  // for (size_t i = 0; i < world.GetSize(); ++i) {
  //   std::cout << "============= Agent # " << i << "=============" << std::endl;
  //   // Run 'erybody for 100 updates.
  //   world[i].SetTrait(5, 0);
  //   world[i].Process(EVAL_TIME);
  //   world[i].PrintProgram();
  //   std::cout << "-- Final State --" << std::endl;
  //   world[i].PrintState();
  // }


  random.Delete();
  return 0;
}
