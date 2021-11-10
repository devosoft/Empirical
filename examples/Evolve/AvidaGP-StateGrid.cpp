/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file AvidaGP-StateGrid.cpp
 *  @brief A example of using AvidaGP evolving with a StateGrid.
 *
 *  Example file of AvidaGP-based organisms (called SGOrg here) moving through a state grid,
 *  trying to consume as many resources as possible without stepping off patches.
 *
 *  States:
 *   -1 = None (poison)
 *    0 = Former food, now eaten.
 *    1 = Current food
 *    2 = Border, no longer has food.
 *    3 = Border, with food.
 */

#include <iostream>

#include "emp/Evolve/StateGrid.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/math/Random.hpp"

class SGOrg : public emp::AvidaCPU_Base<SGOrg> {
protected:
  emp::StateGridStatus sg_status;
  emp::StateGrid state_grid;
  double score;

public:
  using base_t = emp::AvidaCPU_Base<SGOrg>;

  SGOrg() : sg_status(), state_grid(), score(0) { ; }
  SGOrg(emp::Ptr< const emp::AvidaCPU_InstLib<SGOrg> > inst_lib)
    : base_t(inst_lib), sg_status(), state_grid(), score(0) { ; }
  SGOrg(const base_t::genome_t & in_genome) : base_t(in_genome), sg_status(), state_grid(), score(0) { ; }
  SGOrg(const SGOrg &) = default;
  SGOrg(SGOrg &&) = default;

  emp::StateGridStatus & GetSGStatus() { return sg_status; }
  emp::StateGridStatus GetSGStatus() const { return sg_status; }
  emp::StateGrid & GetStateGrid() { return state_grid; }
  const emp::StateGrid & GetStateGrid() const { return state_grid; }
  double GetScore() const { return score; }

  void SetPosition(size_t x, size_t y) { sg_status.SetPos(x,y); }
  void SetFacing(size_t facing) { sg_status.SetFacing(facing); }
  void SetStateGrid(const emp::StateGrid & in_sg) { state_grid = in_sg; }

  double GetFitness() {  // Setup the fitness function.
    ResetHardware();
    Process(200);
    return score;
  }

  void ResetHardware() {
    base_t::ResetHardware();
    score = 0;
  }

  static void Inst_Move(SGOrg & org, const base_t::Instruction & inst) {
    emp_assert(org.state_grid.GetSize() > 0, org.state_grid.GetWidth(), org.state_grid.GetHeight());
    org.sg_status.Move(org.state_grid, org.regs[inst.args[0]]);
  }

  static void Inst_Rotate(SGOrg & org, const base_t::Instruction & inst) {
    org.sg_status.Rotate(org.regs[inst.args[0]]);
  }

  static void Inst_Scan(SGOrg & org, const base_t::Instruction & inst) {
    int val = org.sg_status.Scan(org.state_grid);
    org.regs[inst.args[0]] = val;
    switch (val) {
      case -1: org.score -= 0.5; break;                                            // Poison
      case 0: break;                                                               // Consumed food
      case 1: org.score += 1.0; org.sg_status.SetState(org.state_grid, 0); break;  // Food! (being eaten...)
      case 2: break;                                                               // Empty border
      case 3: org.score += 1.0; org.sg_status.SetState(org.state_grid, 2); break;  // Border w/ food
    }
  }

};

class SGWorld : public emp::World<SGOrg> {
public:
  using inst_lib_t = emp::AvidaCPU_InstLib<SGOrg>;

  inst_lib_t inst_lib;

public:
  SGWorld(emp::Random & random, const std::string & name)
    : emp::World<SGOrg>(random, name), inst_lib()
  {
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

    // OnPlacement( [this](size_t world_id){
    //   pop[world_id]->SetWorldID(world_id);  // Tell organisms their position in environment.
    // } );
  }
  ~SGWorld() { ; }

  const inst_lib_t & GetInstLib() const { return inst_lib; }

};

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

constexpr size_t POP_SIZE = 1000;
constexpr size_t GENOME_SIZE = 50;
constexpr size_t UPDATES = 10000;

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");
  emp::StateGrid state_grid;

  state_grid.AddState(-1, '-', -0.5, "None",          "Empty space; poisonous.");
  state_grid.AddState( 0, '.',  0.0, "Consumed Food", "Previously had sustenance for an organism.");
  state_grid.AddState( 1, '#', +1.0, "Food",          "sustenance to an org.");
  state_grid.AddState( 2, 'x',  0.0, "Consumed Edge", "Edge marker; previously had food.");
  state_grid.AddState( 3, 'X', +1.0, "Edge",          "Edge marker with food.");

  state_grid.Load("state_grids/islands_50x50.cfg");

  // When an organism is added to the world, supply it with a state grid.
  world.OnPlacement( [&state_grid, &world, &random](size_t pos){
    world.GetOrg(pos).SetStateGrid(state_grid);
    // if (pos && random.P(0.1)) world.GetOrg(pos).GetSGStatus().Randomize(state_grid, random);
  } );

  world.SetPopStruct_Mixed(true);

  // Build a random initial population.
  for (size_t i = 0; i < POP_SIZE; i++) {
    SGOrg cpu(&(world.inst_lib));
    cpu.SetStateGrid(state_grid);
    cpu.PushRandom(random, GENOME_SIZE);
    world.Inject(cpu.GetGenome());
  }

  // Setup the mutation function.
  world.SetMutFun( [](SGOrg & org, emp::Random & random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org.RandomizeInst(pos, random);
      }
      return num_muts;
    } );

  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Progress output...
    std::cout << "Update " << ud;

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    std::cout << "  fitness[0] = " << world[0].GetScore()
              << std::endl;

    // Run a tournament for the rest...
    TournamentSelect(world, 4, POP_SIZE-1);

    // Put new organisms in place.
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << "Final Fitness: " << world.CalcFitnessID(0) << std::endl;
  world[0].GetStateGrid().Print();

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
