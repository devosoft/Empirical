#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include "emp/hardware/VirtualCPU.hpp"

#include <sstream>
#include <iostream>

/*
  Variables:
    [ ] NUM_STACKS is actually used
    [ ] Number of registers can be set
    [ ] Number of registers can be changed 
    [ ] Number of nops can be set
    [ ] Number of nops can be changed 
    [ ] We map the nops to their indices (and back) correctly
    [ ] *INSTRUCTIONS struct
      [ ] Constructors assign values correctly
      [ ] Comparison operators work correctly
      [ ] Set functions as expected
      [ ] Defaults???
      [ ] Args???
    [ ] Correct number of registers is initialized 
    [ ] Correct number of inputs are initialized
    [ ] Correct number of outputs are initialized
    [ ] Correct number of stacks are initialized
    [ ] We default to the first stack
    [ ] Heads are initialized to the start of the genome:
      [ ] IP
      [ ] Read 
      [ ] Write
      [ ] Flow
      [ ] Expanded heads
    [ ] Genome is initialized by constructor?
    [ ] Working genome is based on the actual genoem
    [ ] Copied instructions are initially non-existent
    [ ] We have no labels by default
    [ ] Nops need curated by default
    [ ] Default to NON-expanded nop set

   Constructors:
    [ ] Variables are initialized as expected (see above)
    [ ] First constructor sets the genome appropriately
    [ ] Default constructor gives us a default genome (which is???)
    [ ] Copy constructor functions as expected (defaults???)
    [ ] Move constructor functions as expected
   
   Methods:
    [ ] GetSize returns genome size
    [ ] Reset heads resets:
      [ ] Registers
      [ ] Heads
      [ ] Stacks
      [ ] Inputs
      [ ] Outputs
      [ ] Working genome ???
      [ ] Copied instructions
    [ ] Reset
      [ ] ResetHardware
      [ ] Reset genome
      [ ] Reset working genome
      [ ] Labels
    [ ] SetInptuts sets ALL inputs
    [ ] GetOutputs returns ALL outputs 
    [ ] GetInstLib returns correct pointer to instruction library
    [ ] PushInst adds the instruction to the end of the genome AND working genome
      [ ] Update labels?
      [ ] By index 
      [ ] By name
      [ ] By copy
      [ ] By copy (multiple times)
    [ ] PushDefaultInst pushes the first instruction in the library
      [ ] Update labels?
    [ ] ResetIP resets IP to 0
    [ ] AdvanceIP moves IP forward and wraps around genome end if needed
    [ ] SetIP assigns IP to certain location and wraps around genome end if needed
    [ ] ResetRH resets RH to 0
    [ ] AdvanceRH moves RH forward and wraps around genome end if needed
    [ ] ResetRH resets RH to 0
    [ ] AdvanceWH moves WH forward and wraps around genome end if needed
    [ ] ResetWH resets WH to 0
    [ ] AdvanceWH moves WH forward and wraps around genome end if needed
    [ ] ResetFH resets FH to 0
    [ ] AdvanceFH moves FH forward and wraps around genome end if needed
    [ ] SetFH assigns FH to certain location and wraps around genome end if needed
    [ ] ResetModdedHead resets head to 0
    [ ] AdvanceModdedHead moves head forward and wraps around genome end if needed
    [ ] SetModdedHead assigns head to certain location and wraps around genome end if needed
    [ ] GetComplementIdx returns the complement of a single nop 
      [ ] Standard nops
      [ ] Expanded nops
    [ ] GetComplementLabel returns the complemented sequences of nops (rename) 
      [ ] Standard nops
      [ ] Expanded nops
    [ ] CompareSequences determines if two nop vectors are identical
      [ ] Check length too!
    [ ] Find label searches for a nop sequence starting a particular index
    [ ] Find complement searches for the complement of a nop sequence, starting at a given idx
    [ ] CheckIfLastCopied only returns true if the nop vector was the last thing copied
    [ ] CheckIfLastCopiedComplement does ^ but for the complement of the nop vector
    [ ] FindMarkedLabel will search for the nop string at the current genome index
      [ ] start_local parameter will force search to start at current genome index
      [ ] reverse parameter reverses search direction
      [ ] Only looks at labels
    [ ] FindMarkedLabel_Reverse will search backwardfor the nop string at the current 
        genome index
      [ ] start_local parameter will force search to start at current genome index
      [ ] Only looks at labels
    [ ] FindSequences functions as FindLabel but looks at all nops, not just labels
      [ ] start_local parameter will force search to start at current genome index
      [ ] reverse parameter reverses search direction
    [ ] FindSequences_Reverse functions as FindLabel but looks at all nops, not just labels
      [ ] start_local parameter will force search to start at current genome index
      [ ] Only looks at labels
    [ ] StackPush pushes the register value onto the active stack
    [ ] StackPop pops the top value of the active stack and stores it in a register
    [ ] StackSwap actually swaps the active stack
    [ ] CurateNops
      [ ] Counts nops if needed
      [ ] Finds all labels
      [ ] Add nops to preceeding instructions nop_vec
      [ ] Wraps?
      [ ] Sets boolean flag to false
    [ ] CountNops
      [ ] Calculates the number of nops
      [ ] Maps nop ids to indices
      [ ] Maps nop indices to ids
    [ ] ExpandRegisters
      [ ] Count nops if needed
      [ ] Sets num_regs variable
      [ ] Resizes register vector
    [ ] SingleProcess
      [ ] Do bookkeeping as needed based on flags
      [ ] Process the next instruction
      [ ] Auto-advance IP?
    [ ] Process
      [ ] Calls SingleProcess N times
    [ ] GetRandomInst returns a random instruction within the instruction library
    [ ] SetInst overwrites an instruction in the genome/working genome
      [ ] Recalcuates labels/nops?
    [ ] RandomizeInst calls SetInst on the genome position, but with a random instruction
    [ ] GetString returns a string representation of the working genome
    [ ] GetOriginalString returns a string representation of the (non-working) genome
    [ ] Print details dumps registers and such to the stream
    [ ] Load 
      [ ] Loads genome from a stream
      [ ] That stream can be a file
*/


// VirtualCPU is currently constructed to always be derived from.
// Here we create an empty derived class
class Derived : public emp::VirtualCPU<Derived> { };

TEST_CASE("VirtualCPU", "[Hardware]") {
  for(size_t idx = 0; idx < 23; ++idx)
    std::cout << (std::string)"Nop" + (char)('A' + idx) << std::endl;
  //Derived cpu;
  //CHECK(cpu.stacks.size() == cpu.NUM_STACKS);
  //for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
  //  CHECK(cpu.stacks[idx].empty());
  //}
  //CHECK(cpu.GetNumRegs() == 3); // defaults to 3 from the original avida architecture
  //CHECK(cpu.regs.size() == cpu.GetNumRegs());

/*
  Variables:
    [X] NUM_STACKS is actually used
    [X] Stacks are intially empty
    [ ] Number of registers can be set
    [ ] Number of registers can be changed 
    [ ] Number of nops can be set
    [ ] Number of nops can be changed 
    [ ] We map the nops to their indices (and back) correctly
    [ ] *INSTRUCTIONS struct
      [ ] Constructors assign values correctly
      [ ] Comparison operators work correctly
      [ ] Set functions as expected
      [ ] Defaults???
      [ ] Args???
    [ ] Correct number of registers is initialized 
    [ ] Correct number of inputs are initialized
    [ ] Correct number of outputs are initialized
    [ ] Correct number of stacks are initialized
    [ ] We default to the first stack
    [ ] Heads are initialized to the start of the genome:
      [ ] IP
      [ ] Read 
      [ ] Write
      [ ] Flow
      [ ] Expanded heads
    [ ] Genome is initialized by constructor?
    [ ] Working genome is based on the actual genoem
    [ ] Copied instructions are initially non-existent
    [ ] We have no labels by default
    [ ] Nops need curated by default
    [ ] Default to NON-expanded nop set
*/

	//REQUIRE(gp.GetInstLib() == gp2.GetInstLib());
	//REQUIRE(gp.GetSize() == 0);
	//REQUIRE(gp.GetIP() == 0);
	//REQUIRE(gp.GetInput(3) == 0.0);
	//REQUIRE(gp.GetOutput(7) == 0.0);
	//REQUIRE(gp.GetNumInputs() == 0.0);
	//REQUIRE(gp.GetNumOutputs() == 0.0);
	//REQUIRE(gp.GetNumTraits() == 0.0);
	//REQUIRE(gp.GetNumErrors() == 0.0);

	//// instructions
	//gp.PushInst(0, 0);
	//gp.PushInst("Dec", 1);
	//REQUIRE(gp.GetInst(0).id == 0);
	//REQUIRE(gp.GetInst(1).id == 1);
	//gp.SetInst(0, 2, 0);
	//REQUIRE(gp.GetInst(0).id == 2);

	//// traits
	//gp.PushTrait(4.0);
	//REQUIRE(gp.GetNumTraits() == 1);
	//REQUIRE(gp.GetTrait(0) == 4.0);
	//gp.PushTrait(2.0);
	//gp.PushTrait(3.0);
	//emp::vector<double> traits = gp.GetTraits();
	//for(size_t i=0; i<traits.size();i++)
	//{
	//	if(std::fmod(traits[i],2) != 0.0)
	//	{
	//		gp.SetTrait(i, traits[i]*2.0);
	//	}
	//}
	//REQUIRE(gp.GetTrait(2) == 6.0);

	//gp.IncErrors();
	//REQUIRE(gp.GetNumErrors() == 1);

	//// RandomizeInst PushRandom
	//emp::Random rnd(1200);
	//gp.PushRandom(rnd);
	//REQUIRE(gp.GetGenome().size() == 3);
	//gp.RandomizeInst(0, rnd);
	//if(gp.GetInst(0).id != 2)
	//{
	//	// on clang, random seed doesn't create the same instructions
	//	gp.SetInst(0, 2, 4, 10, 15);
	//	gp.SetInst(2, 2, 9, 8, 13);
	//}
	//REQUIRE(gp.GetInst(0).id == 2); // !4 10 15
	//REQUIRE(gp.GetInst(1).id == 1);
	//REQUIRE(gp.GetInst(2).id == 2); // !9 8 13

	//// SingleProcess
	//REQUIRE(gp.GetReg(9) == 9.0);
	//gp.SetIP(2);
	//gp.SingleProcess();
	//REQUIRE(gp.GetReg(9) == 0.0);

	//// ProcessInst
	//REQUIRE(gp.GetReg(4) == 4.0);
	//gp.ProcessInst(gp.GetInst(0));
	//REQUIRE(gp.GetReg(4.0) == 0.0);
	//gp.ProcessInst(gp.GetInst(0));
	//REQUIRE(gp.GetReg(4.0) == 1.0);

	//// Inputs
	//std::unordered_map<int, double> inpts({{0, 2.0},{1, 6.0},{2, 34.0}});
	//gp.SetInputs(inpts);
	//REQUIRE(gp.GetNumInputs() == 3);
	//REQUIRE(gp.GetInputs() == inpts);
	//REQUIRE(gp.GetInput(0) == 2.0);
	//gp.SetInput(0, 46.0);
	//REQUIRE(gp.GetInput(0) == 46.0);

	//// Outputs
	//std::unordered_map<int, double> otpts({{0, 72.0},{1, 8.0},{2, 18.0}});
	//gp.SetOutputs(otpts);
	//REQUIRE(gp.GetNumOutputs() == 3);
	//REQUIRE(gp.GetOutputs() == otpts);
	//REQUIRE(gp.GetOutput(0) == 72.0);
	//gp.SetOutput(0, 22.0);
	//REQUIRE(gp.GetOutput(0) == 22.0);
}
