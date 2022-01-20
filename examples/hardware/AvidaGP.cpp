/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017
 *
 *  @file AvidaGP.cpp
 *  @brief This is example code for using AvidaGP.
 */

#include <iostream>

#include "emp/hardware/AvidaGP.hpp"
#include "emp/hardware/InstLib.hpp"
#include "emp/math/Random.hpp"

void Print(const emp::AvidaGP & cpu) {
  for (size_t i = 0; i < 16; i++) {
    std::cout << "[" << cpu.GetReg(i) << "] ";
  }
  std::cout << " IP=" << cpu.GetIP() << std::endl;
}

int main()
{
  std::cout << "Test." << std::endl;

  // const auto & inst_lib = emp::AvidaGP::GetInstLib();
  emp::Random random;

  emp::AvidaGP cpu;
  Print(cpu);
  cpu.PushInst( "Countdown", 8, 4 );
  cpu.PushInst( "Mult", 6, 2, 6 );
  cpu.PushInst( "Scope", 0 );

  cpu.PushInst( "Inc", 1 );
  cpu.PushInst( "Inc", 1 );
  cpu.PushInst( "Mult", 1, 2, 3 );
  cpu.PushInst( "Add", 3, 4, 5 );

  for (int i = 0; i < 43; i++) cpu.PushRandom(random);

  for (size_t i = 0; i < 100; i++) {
    cpu.Process(1);
    Print(cpu);
  }

  cpu.PrintGenome();
  cpu.PrintGenome("test.org");
  cpu.ResetHardware();
  cpu.Trace(200);

  emp::AvidaGP cpu2;
  cpu2.Load("test.org");

  std::cout << "Original CPU size: " << cpu.GetSize() << std::endl;
  std::cout << "Reloaded CPU size: " << cpu2.GetSize() << std::endl;

  // Run a ton of organisms.
  for (int t = 0; t < 1000000; t++) {
    if (t % 10000 == 0) std::cout << t << std::endl;
    cpu.Reset();
    for (int i = 0; i < 100; i++) cpu.PushRandom(random);
    cpu.Process(200);
  }
  std::cout << "NEW AvidaGP" << std::endl;

  return 0;
}
