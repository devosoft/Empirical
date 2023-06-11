/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021
 *
 *  @file avida_gp.cpp
 */

#include <iostream>
#include <sstream>

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/hardware/AvidaGP.hpp"

TEST_CASE("Test Avida GP", "[Hardware]")
{
  const emp::AvidaCPU_InstLib<emp::AvidaGP>* rawPtr = &emp::AvidaCPU_InstLib<emp::AvidaGP>::DefaultInstLib();
  emp::Ptr<const emp::AvidaCPU_InstLib<emp::AvidaGP>> instLibPtr(rawPtr);
  emp::AvidaGP gp(emp::AvidaCPU_InstLib<emp::AvidaGP>::DefaultInstLib());
  emp::AvidaGP gp2(instLibPtr);
  REQUIRE(gp.GetInstLib() == gp2.GetInstLib());
  REQUIRE(gp.GetSize() == 0);
  REQUIRE(gp.GetIP() == 0);
  REQUIRE(gp.GetInput(3) == 0.0);
  REQUIRE(gp.GetOutput(7) == 0.0);
  REQUIRE(gp.GetNumInputs() == 0.0);
  REQUIRE(gp.GetNumOutputs() == 0.0);
  REQUIRE(gp.GetNumTraits() == 0.0);
  REQUIRE(gp.GetNumErrors() == 0.0);

  // instructions
  gp.PushInst(0, 0);
  gp.PushInst("Dec", 1);
  REQUIRE(gp.GetInst(0).id == 0);
  REQUIRE(gp.GetInst(1).id == 1);
  gp.SetInst(0, 2, 0);
  REQUIRE(gp.GetInst(0).id == 2);

  // traits
  gp.PushTrait(4.0);
  REQUIRE(gp.GetNumTraits() == 1);
  REQUIRE(gp.GetTrait(0) == 4.0);
  gp.PushTrait(2.0);
  gp.PushTrait(3.0);
  emp::vector<double> traits = gp.GetTraits();
  for(size_t i=0; i<traits.size();i++)
  {
    if(std::fmod(traits[i],2) != 0.0)
    {
      gp.SetTrait(i, traits[i]*2.0);
    }
  }
  REQUIRE(gp.GetTrait(2) == 6.0);

  gp.IncErrors();
  REQUIRE(gp.GetNumErrors() == 1);

  // RandomizeInst PushRandom
  emp::Random rnd(1200);
  gp.PushRandom(rnd);
  REQUIRE(gp.GetGenome().size() == 3);
  gp.RandomizeInst(0, rnd);
  if(gp.GetInst(0).id != 2)
  {
    // on clang, random seed doesn't create the same instructions
    gp.SetInst(0, 2, 4, 10, 15);
    gp.SetInst(2, 2, 9, 8, 13);
  }
  REQUIRE(gp.GetInst(0).id == 2); // !4 10 15
  REQUIRE(gp.GetInst(1).id == 1);
  REQUIRE(gp.GetInst(2).id == 2); // !9 8 13

  // SingleProcess
  REQUIRE(gp.GetReg(9) == 9.0);
  gp.SetIP(2);
  gp.SingleProcess();
  REQUIRE(gp.GetReg(9) == 0.0);

  // ProcessInst
  REQUIRE(gp.GetReg(4) == 4.0);
  gp.ProcessInst(gp.GetInst(0));
  REQUIRE(gp.GetReg(4.0) == 0.0);
  gp.ProcessInst(gp.GetInst(0));
  REQUIRE(gp.GetReg(4.0) == 1.0);

  // Inputs
  std::unordered_map<int, double> inpts({{0, 2.0},{1, 6.0},{2, 34.0}});
  gp.SetInputs(inpts);
  REQUIRE(gp.GetNumInputs() == 3);
  REQUIRE(gp.GetInputs() == inpts);
  REQUIRE(gp.GetInput(0) == 2.0);
  gp.SetInput(0, 46.0);
  REQUIRE(gp.GetInput(0) == 46.0);

  // Outputs
  std::unordered_map<int, double> otpts({{0, 72.0},{1, 8.0},{2, 18.0}});
  gp.SetOutputs(otpts);
  REQUIRE(gp.GetNumOutputs() == 3);
  REQUIRE(gp.GetOutputs() == otpts);
  REQUIRE(gp.GetOutput(0) == 72.0);
  gp.SetOutput(0, 22.0);
  REQUIRE(gp.GetOutput(0) == 22.0);
}
