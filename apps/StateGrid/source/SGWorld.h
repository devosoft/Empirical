/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  SGWorld.h
 *  @brief A world a population of SGOrgs.
 */

#include "Evolve/StateGrid.h"
#include "Evolve/World.h"
#include "tools/Random.h"

#include "SGOrg.h"
#include "SGPatches.h"

class SGWorld : public emp::World<SGOrg> {
public:
  using inst_lib_t = emp::AvidaCPU_InstLib<SGOrg>;

  inst_lib_t inst_lib;
  SGPatches state_grid;
  emp::vector< std::function<double(SGOrg &)> > hint_funs;

  size_t POP_SIZE;         ///< Total population size.
  size_t GENOME_SIZE;      ///< How long of a genome should we be using?
  size_t CPU_TIME;         ///< How many CPU cycles to process for?
  double GOOD_BAD_RATIO;   ///< Value of going to a good square vs avoiding a bad square.
  size_t ELITE_SIZE;       ///< Top how many organisms should move to next generation?
  size_t ELITE_COPIES;     ///< How many copies of each elite organism should be made?
  size_t ELITE_TOTAL;      ///< Total number of individuals produced by elite selection.

public:
  SGWorld(emp::Random & random, const std::string & name)
    : emp::World<SGOrg>(random, name)
    , inst_lib()
    , state_grid()
    , hint_funs()
  {
    POP_SIZE = 400;
    GENOME_SIZE = 50;
    CPU_TIME = 5000;
    GOOD_BAD_RATIO = 1.0;
    ELITE_SIZE = 10;
    ELITE_COPIES = 1;
    ELITE_TOTAL = ELITE_SIZE * ELITE_COPIES;

    // Build the instruction library...
    inst_lib.AddInst("Inc", inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
    inst_lib.AddInst("Dec", inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
    inst_lib.AddInst("Not", inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
    inst_lib.AddInst("SetReg", inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
    inst_lib.AddInst("Add", inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
    inst_lib.AddInst("Sub", inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
    inst_lib.AddInst("Mult", inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
    inst_lib.AddInst("Div", inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
    inst_lib.AddInst("Mod", inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
    inst_lib.AddInst("TestEqu", inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
    inst_lib.AddInst("TestNEqu", inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
    inst_lib.AddInst("TestLess", inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
    inst_lib.AddInst("If", inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
    inst_lib.AddInst("While", inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Countdown", inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Break", inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
    inst_lib.AddInst("Scope", inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
    inst_lib.AddInst("Define", inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
    inst_lib.AddInst("Call", inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
    inst_lib.AddInst("Push", inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
    inst_lib.AddInst("Pop", inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
    inst_lib.AddInst("Input", inst_lib_t::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
    inst_lib.AddInst("Output", inst_lib_t::Inst_Output, 2, "Push reg Arg1 into output Arg2");
    inst_lib.AddInst("CopyVal", inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
    inst_lib.AddInst("ScopeReg", inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

    inst_lib.AddInst("Move",   SGOrg::Inst_Move,   1, "Move forward in state grid.");
    inst_lib.AddInst("Rotate", SGOrg::Inst_Rotate, 1, "Rotate in place in state grid.");
    inst_lib.AddInst("Scan",   SGOrg::Inst_Scan,   1, "Idenify state of current position in state grid.");

    // When an organism is added to the world, supply it with a state grid.
    OnPlacement( [this, &random](size_t pos) {
      pop[pos]->SetStateGrid(state_grid);      // Ensure new org has access to state grid.
      pop[pos]->GetSGStatus().SetPos(4,4);     // Start at position (4,4)
      pop[pos]->GetSGStatus().TrackMoves();    // Make sure we have a history to refer to.

      // Setup a random position and rectilinear facing.
      if (pos) {
        GetOrg(pos).GetSGStatus().Randomize(state_grid, random);
        GetOrg(pos).GetSGStatus().SetFacing(random.GetUInt(4)*2+1);
      }
    } );

    // Setup the MUTATION function.
    SetMutFun( [](SGOrg & org, emp::Random & random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(org.genome.sequence.size());
        org.RandomizeInst(pos, random);
      }
      return num_muts;
    } );

    // Setup the basic FITNESS function
    emp::BitVector good_sites = state_grid.IsState(1);
    emp::BitVector bad_sites = state_grid.IsState(-1);
    auto fit_fun = [good_sites, bad_sites](const SGOrg & org) {
      emp::BitVector visited_sites = org.GetVisited();
      double good_visits = (good_sites & visited_sites).CountOnes();
      double bad_visits = (bad_sites & visited_sites).CountOnes();
      double result = good_visits - bad_visits;
      return emp::Max(0.0, result);
    };
    SetFitFun(fit_fun);

    // Setup a set of HINT fitness functions.
    for (size_t h = 0; h < state_grid.GetSize(); h++) {
      int target_state = state_grid.GetState(h);
      if (target_state != -1 && target_state != 1) continue;
      size_t target_x = h % state_grid.GetWidth();
      size_t target_y = h / state_grid.GetWidth();

      hint_funs.push_back( [target_x,target_y,target_state](SGOrg & org) {
        return org.GetSGStatus().WasAt(target_x, target_y) ? target_state : 0.0;
      });
    }

    // Setup a well-mixed population structure
    SetPopStruct_Mixed(true);

    // Build a random initial popoulation.
    for (size_t i = 0; i < POP_SIZE; i++) {
      SGOrg cpu(&inst_lib);
      cpu.PushRandom(random, GENOME_SIZE);
      Inject(cpu.GetGenome());
    }

  }
  ~SGWorld() { ; }

  void RunUpdate() {
    // Run all of the organisms to trace their movement.
    ResetHardware();
    Process(CPU_TIME);

    // Keep the best individual.
    EliteSelect(*this, ELITE_SIZE, ELITE_COPIES);

    // // Run a tournament for the rest...
    // TournamentSelect(world, 4, POP_SIZE-ELITE_TOTAL);

    // Run Lexicase selection for the rest...
    LexicaseSelect(*this, hint_funs, POP_SIZE-ELITE_TOTAL);

    // Put new organisms is place.
    Update();

    // Mutate all but the first organism.
    DoMutations(1);
  }

  const inst_lib_t & GetInstLib() const { return inst_lib; }

  SGPatches & GetStateGrid() { return state_grid; }
  const SGPatches & GetStateGrid() const { return state_grid; }
};
