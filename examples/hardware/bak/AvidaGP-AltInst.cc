//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include <iostream>

#include "hardware/AvidaGP.h"
#include "hardware/InstLib.h"
#include "tools/Random.h"
#include "evo/World.h"

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
  // Build the instruction library.
  emp::InstLib<emp::AvidaGP> inst_lib;
  inst_lib.AddInst("Inc", emp::AvidaGP::Inst_Inc, 1, "Increment value in reg Arg1");
  inst_lib.AddInst("Dec", emp::AvidaGP::Inst_Dec, 1, "Decrement value in reg Arg1");
  inst_lib.AddInst("Not", emp::AvidaGP::Inst_Not, 1, "Logically toggle value in reg Arg1");
  inst_lib.AddInst("SetReg", emp::AvidaGP::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
  inst_lib.AddInst("Add", emp::AvidaGP::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
  inst_lib.AddInst("Sub", emp::AvidaGP::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
  inst_lib.AddInst("Mult", emp::AvidaGP::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
  inst_lib.AddInst("Div", emp::AvidaGP::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
  inst_lib.AddInst("Mod", emp::AvidaGP::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
  inst_lib.AddInst("While", emp::AvidaGP::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
  inst_lib.AddInst("Countdown", emp::AvidaGP::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
  inst_lib.AddInst("Break", emp::AvidaGP::Inst_Break, 1, "Break out of scope Arg1");
  inst_lib.AddInst("Scope", emp::AvidaGP::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
  inst_lib.AddInst("Input", emp::AvidaGP::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
  inst_lib.AddInst("Output", emp::AvidaGP::Inst_Output, 2, "Push reg Arg1 into output Arg2");
  inst_lib.AddInst("CopyVal", emp::AvidaGP::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
  inst_lib.AddInst("ScopeReg", emp::AvidaGP::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

  auto square_fun = [](emp::AvidaGP & hw, const emp::AvidaGP::arg_set_t & args) {
    double val = hw.GetReg(args[0]);
    hw.SetReg(args[0], val*val);
  };
  inst_lib.AddInst("Square", square_fun, 1, "Square the value in the specified register.");

  emp::Random random;
  emp::evo::EAWorld<emp::AvidaGP> world(random, "AvidaWorld");

  // Build a random initial popoulation.
  for (size_t i = 0; i < POP_SIZE; i++) {
    emp::AvidaGP cpu(inst_lib);
    cpu.PushRandom(random, GENOME_SIZE);
    world.Insert(cpu);
  }

  // Setup the mutation function.
  world.SetDefaultMutateFun( [](emp::AvidaGP* org, emp::Random& random) {
      uint32_t num_muts = random.GetUInt(4);  // 0 to 3 mutations.
      for (uint32_t m = 0; m < num_muts; m++) {
        const uint32_t pos = random.GetUInt(GENOME_SIZE);
        org->RandomizeInst(pos, random);
      }
      return (num_muts > 0);
    } );

  // Setup the fitness function.
  std::function<double(emp::AvidaGP*)> fit_fun =
    [](emp::AvidaGP * org) {
      int count = 0;
      for (int i = 0; i < 16; i++) {
        if (org->GetOutput(i) == (double) (i*i)) count++;
      }
      return (double) count;
    };

  emp::vector< std::function<double(emp::AvidaGP*)> > fit_set(16);
  for (size_t out_id = 0; out_id < 16; out_id++) {
    // Setup the fitness function.
    fit_set[out_id] = [out_id](emp::AvidaGP * org) {
      return (double) -std::abs(org->GetOutput((int)out_id) - (double) (out_id * out_id));
    };
  }


  // Do the run...
  for (size_t ud = 0; ud < UPDATES; ud++) {
    // Update the status of all organisms.
    for (size_t id = 0; id < POP_SIZE; id++) {
      world[id].ResetHardware();
      world[id].Process(200);
    }

    // Keep the best individual.
    world.EliteSelect(fit_fun, 1, 1);

    // Run a tournament for the rest...
    // world.TournamentSelect(fit_fun, 5, POP_SIZE-1);
    world.LexicaseSelect(fit_set, POP_SIZE-1);
    // world.EcoSelect(fit_fun, fit_set, 100, 5, POP_SIZE-1);
    world.Update();
    std::cout << (ud+1) << " : " << 0 << " : " << fit_fun(&(world[0])) << std::endl;

    // Mutate all but the first organism.
    world.MutatePop(1);
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
