/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  AvidaGP-StateGrid.cc
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

#include "hardware/AvidaGP.h"
#include "hardware/InstLib.h"
#include "tools/Random.h"
#include "Evo/StateGrid.h"
#include "Evo/World.h"

class SGOrg : public emp::AvidaGP {
protected:
  emp::StateGridStatus sg_status;
  emp::StateGrid state_grid;
  double score;

public:
  SGOrg() : sg_status(), state_grid(), score(0) { ; }
  SGOrg(const emp::AvidaGP::genome_t & in_genome) : AvidaGP(in_genome), sg_status(), state_grid(), score(0) { ; }
  SGOrg(const SGOrg &) = default;
  SGOrg(SGOrg &&) = default;

  emp::StateGridStatus GetSGStatus() const { return sg_status; }

  void SetPosition(size_t x, size_t y) { sg_status.SetPos(x,y); }
  void SetFacing(size_t facing) { sg_status.SetFacing(facing); }
  void SetStateGrid(const emp::StateGrid & in_sg) { state_grid = in_sg; }

  double GetFitness() {  // Setup the fitness function.
    ResetHardware();
    Process(200);
    return score;
  }

  void ResetHardware() {
    emp::AvidaGP::ResetHardware();
    score = 0;
  }

  static void Inst_Move(SGOrg & hw, const emp::AvidaGP::Instruction & inst) {
    hw.sg_status.Move(hw.state_grid, hw.regs[inst.args[0]]);
  }

  static void Inst_Rotate(SGOrg & hw, const emp::AvidaGP::Instruction & inst) {
    hw.sg_status.Rotate(hw.regs[inst.args[0]]);
  }

  static void Inst_Scan(SGOrg & hw, const emp::AvidaGP::Instruction & inst) {
    int val = hw.sg_status.Scan(hw.state_grid);
    hw.regs[inst.args[0]] = val;
    switch (val) {
      case -1: hw.score -= 0.5; break;                                     // Poison
      case 0: break;                                                       // Eaten food
      case 1: hw.score += 1.0; hw.sg_status.Set(hw.state_grid, 0); break;  // Food! (being eaten...)
      case 2: break;                                                       // Empty border
      case 3: hw.score += 1.0; hw.sg_status.Set(hw.state_grid, 2); break;  // Border with food.
    }
  }

};

class SGWorld : public emp::World<SGOrg> {
public:
  using inst_lib_t = emp::InstLib<SGOrg>;

protected:
  inst_lib_t inst_lib;

public:
  SGWorld(emp::Random & random, const std::string & name)
    : emp::World<SGOrg>(random, name), inst_lib()
  {
    // Build the instruction library...
    inst_lib.AddInst("Inc", emp::AvidaGP::Inst_Inc, 1, "Increment value in reg Arg1");
    inst_lib.AddInst("Dec", emp::AvidaGP::Inst_Dec, 1, "Decrement value in reg Arg1");
    inst_lib.AddInst("Not", emp::AvidaGP::Inst_Not, 1, "Logically toggle value in reg Arg1");
    inst_lib.AddInst("SetReg", emp::AvidaGP::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
    inst_lib.AddInst("Add", emp::AvidaGP::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
    inst_lib.AddInst("Sub", emp::AvidaGP::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
    inst_lib.AddInst("Mult", emp::AvidaGP::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
    inst_lib.AddInst("Div", emp::AvidaGP::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
    inst_lib.AddInst("Mod", emp::AvidaGP::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
    inst_lib.AddInst("TestEqu", emp::AvidaGP::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
    inst_lib.AddInst("TestNEqu", emp::AvidaGP::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
    inst_lib.AddInst("TestLess", emp::AvidaGP::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
    inst_lib.AddInst("If", emp::AvidaGP::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
    inst_lib.AddInst("While", emp::AvidaGP::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Countdown", emp::AvidaGP::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
    inst_lib.AddInst("Break", emp::AvidaGP::Inst_Break, 1, "Break out of scope Arg1");
    inst_lib.AddInst("Scope", emp::AvidaGP::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
    inst_lib.AddInst("Define", emp::AvidaGP::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
    inst_lib.AddInst("Call", emp::AvidaGP::Inst_Call, 1, "Call previously defined function Arg1");
    inst_lib.AddInst("Push", emp::AvidaGP::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
    inst_lib.AddInst("Pop", emp::AvidaGP::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
    inst_lib.AddInst("Input", emp::AvidaGP::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
    inst_lib.AddInst("Output", emp::AvidaGP::Inst_Output, 2, "Push reg Arg1 into output Arg2");
    inst_lib.AddInst("CopyVal", emp::AvidaGP::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
    inst_lib.AddInst("ScopeReg", emp::AvidaGP::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

    inst_lib.AddInst("Move",   SGOrg::Inst_Move,   1, "Move forward in state grid.");
    inst_lib.AddInst("Rotate", SGOrg::Inst_Rotate, 1, "Rotate in place in state grid.");
    inst_lib.AddInst("Scan",   SGOrg::Inst_Scan,   1, "Idenify state of current position in state grid.");

    // OnOrgPlacement( [this](size_t world_id){
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
constexpr size_t UPDATES = 500;

int main()
{
  emp::Random random;
  SGWorld world(random, "AvidaWorld");
  world.SetWellMixed(true);

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    SGOrg cpu;
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
    std::cout << "Update " << ud << std::endl;

    // Keep the best individual.
    EliteSelect(world, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(world, 5, POP_SIZE-1);

    // Put new organisms is place.
    world.Update();

    // Mutate all but the first organism.
    world.DoMutations(1);
  }

  std::cout << std::endl;
  world[0].PrintGenome();
  std::cout << std::endl;
  for (int i = 0; i < 16; i++) {
    std::cout << i << ":" << world[0].GetOutput(i) << "  ";
  }
  std::cout << std::endl;

  return 0;
}
