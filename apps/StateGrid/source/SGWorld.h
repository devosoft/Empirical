/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file  SGWorld.h
 *  @brief A world a population of SGOrgs.
 */

#include "Evo/StateGrid.h"
#include "Evo/World.h"
#include "tools/Random.h"

#include "SGOrg.h"
#include "SGPatches.h"

class SGWorld : public emp::World<SGOrg> {
 public:
  using inst_lib_t = emp::AvidaCPU_InstLib<SGOrg>;

  inst_lib_t inst_lib;
  SGPatches state_grid;

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

      // When an organism is added to the world, supply it with a state grid.
      OnOrgPlacement( [this, &random](size_t pos){
          pop[pos]->SetStateGrid(state_grid);
          // if (pos && random.P(0.1)) world.GetOrg(pos).GetSGStatus().Randomize(state_grid, random);
        } );

      // Setup the mutation function.
      SetMutFun( [](SGOrg & org, emp::Random & random) {
          uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
          for (uint32_t m = 0; m < num_muts; m++) {
            const uint32_t pos = random.GetUInt(org.genome.sequence.size());
            org.RandomizeInst(pos, random);
          }
          return num_muts;
        } );
    }
  ~SGWorld() { ; }

  const inst_lib_t & GetInstLib() const { return inst_lib; }

};
