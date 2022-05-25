#define CATCH_CONFIG_MAIN

#include "third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>
#include <sstream>

#include "emp/hardware/VirtualCPU.hpp"

/* TODO
  [ ] *INSTRUCTIONS struct
    [ ] Constructors assign values correctly
    [ ] Comparison operators work correctly
    [ ] Set functions as expected
    [ ] Defaults???
    [ ] Args???
  [ ] Expanded heads???
*/

// VirtualCPU is currently constructed to always be derived from.
// Here we create an empty derived class
class Derived : public emp::VirtualCPU<Derived> {
  public:
    using base_t = emp::VirtualCPU<Derived>;
    Derived() : base_t() { ; }
    Derived(const Derived::genome_t& genome) : base_t(genome) { ; }
    Derived(const Derived& other) : base_t(other) { ; }
    Derived(const Derived&& other) : base_t(other) { ; }
};
    
Derived CreateSeedCPU(){
  Derived cpu_init;
  emp::Random random(56);
  for(size_t i = 0; i < 10; ++i)
    cpu_init.PushRandomInst(random);
  // Make a change to the working genome
  while(cpu_init.genome[0] == cpu_init.genome_working[0])
    cpu_init.genome_working[0] = cpu_init.GetRandomInst(random);
  CHECK(cpu_init.genome.size() == 10);
  CHECK(cpu_init.genome_working.size() == 10);
  CHECK(7 == (cpu_init.inst_ptr = 7));
  CHECK(3 == (cpu_init.read_head = 3));
  CHECK(4 == (cpu_init.write_head = 4));
  cpu_init.nop_id_set.insert(4);
  CHECK(cpu_init.nop_id_set.size() == 4);
  cpu_init.label_idx_vec.push_back(3);
  CHECK(2 == (cpu_init.regs[0] = 2));
  CHECK(38 == (cpu_init.regs[0] = 38));
  CHECK(2309 == (cpu_init.regs[0] = 2309));
  CHECK(5 == (cpu_init.inputs[0] = 5));
  CHECK(2 == (cpu_init.outputs[0] = 2));
  CHECK(1 == (cpu_init.active_stack_idx = 1));
  cpu_init.stacks[cpu_init.active_stack_idx].push_back(1); 
  CHECK(cpu_init.stacks[cpu_init.active_stack_idx].size() == 1);
  return cpu_init;
}

TEST_CASE("VirtualCPU_Variables", "[Hardware]") {
  Derived cpu;
  CHECK(cpu.stacks.size() == cpu.NUM_STACKS); // NUM_STACKS is actually used
  // All stacks are initially empty
  for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
    CHECK(cpu.stacks[idx].empty());
  }
  CHECK(cpu.active_stack_idx == 0);   // Default to first stack
  CHECK(cpu.GetNumNops() == 3);       // Default instruction set has 3 nops
  CHECK(cpu.nop_id_set.size() == 3);  // All three default nops present in set
  CHECK(emp::Has(cpu.nop_id_set, 0)); // NopA in set
  CHECK(emp::Has(cpu.nop_id_set, 1)); // NopB in set
  CHECK(emp::Has(cpu.nop_id_set, 2)); // NopC in set
  CHECK(cpu.GetNumRegs() == 3); // 3 nops in instruction set forces cpu to have 3 registers 
  // All registers start at their index value 
  for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
    CHECK(cpu.regs[idx] == idx);
  }
  CHECK(cpu.inputs.size() == 0);  // Start with no inputs
  CHECK(cpu.outputs.size() == 0); // Start with no outputs
  CHECK(cpu.inst_ptr == 0);   // All heads default to 0
  CHECK(cpu.read_head == 0);  // All heads default to 0
  CHECK(cpu.write_head == 0); // All heads default to 0
  CHECK(cpu.flow_head == 0);  // All heads default to 0
  CHECK(cpu.copied_inst_id_vec.size() == 0); // No instructions copied
  CHECK(cpu.label_idx_vec.size() == 0); // No labels in empty genome
  CHECK(cpu.nops_need_curated == true); // Nops are NOT curated at initialization 
  CHECK(cpu.expanded_nop_args == false);// Nop arguments are NOT expanded by default 
  CHECK(cpu.are_nops_counted == true);  // Defaults to false but flipped during initialization
  CHECK(cpu.are_regs_expanded == true); // Defaults to false but flipped during initialization
  /*
    [X] NUM_STACKS is actually used
    [X] Stacks are initially empty 
    [X] We store all the nops in the nop set
    [X] Correct number of registers is initialized 
    [X] Correct number of inputs are initialized
    [X] Correct number of outputs are initialized
    [X] Correct number of stacks are initialized
    [X] We default to the first stack
    [X] Heads are initialized to the start of the genome:
      [X] IP
      [X] Read 
      [X] Write
      [X] Flow
      [ ] Expanded heads
    [x] Copied instructions are initially non-existent
    [X] We have no labels by default
    [X] Nops need curated by default
    [X] Default to NON-expanded nop set
    [X] are_nops_counted set true during initialization
    [X] are_regs_expanded set true during initialization
  */
}
TEST_CASE("VirtualCPU_Constructors", "[Hardware]") {
  { // Default constructor
    Derived cpu;
    CHECK(cpu.GetInstLib().Raw() == &Derived::inst_lib_t::DefaultInstLib()); 
    CHECK(cpu.GetGenomeSize() == 0);
    CHECK(cpu.GetWorkingGenomeSize() == 0);
    // VARIABLES -- should default
    {
      CHECK(cpu.stacks.size() == cpu.NUM_STACKS); // NUM_STACKS is actually used
      // All stacks are initially empty
      for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
        CHECK(cpu.stacks[idx].empty());
      }
      CHECK(cpu.active_stack_idx == 0);   // Default to first stack
      CHECK(cpu.GetNumNops() == 3);       // Default instruction set has 3 nops
      CHECK(cpu.nop_id_set.size() == 3);  // All three default nops present in set
      CHECK(emp::Has(cpu.nop_id_set, 0)); // NopA in set
      CHECK(emp::Has(cpu.nop_id_set, 1)); // NopB in set
      CHECK(emp::Has(cpu.nop_id_set, 2)); // NopC in set
      CHECK(cpu.GetNumRegs() == 3); // 3 nops in instruction set forces cpu to have 3 registers 
      // All registers start at their index value 
      for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
        CHECK(cpu.regs[idx] == idx);
      }
      CHECK(cpu.inputs.size() == 0);  // Start with no inputs
      CHECK(cpu.outputs.size() == 0); // Start with no outputs
      CHECK(cpu.inst_ptr == 0);   // All heads default to 0
      CHECK(cpu.read_head == 0);  // All heads default to 0
      CHECK(cpu.write_head == 0); // All heads default to 0
      CHECK(cpu.flow_head == 0);  // All heads default to 0
      CHECK(cpu.copied_inst_id_vec.size() == 0); // No instructions copied
      CHECK(cpu.label_idx_vec.size() == 0); // No labels in empty genome
      CHECK(cpu.nops_need_curated == true); // Nops are NOT curated at initialization 
      CHECK(cpu.expanded_nop_args == false);// Nop arguments are NOT expanded by default 
      CHECK(cpu.are_nops_counted == true);  // Defaults to false but flipped during initialization
      CHECK(cpu.are_regs_expanded == true); // Defaults to false but flipped during initialization
    }
  }
  { // Genome constructor
    // Create default CPU to steal its instruction library
    Derived cpu_init;
    // Create a genome with ten random instructions
    Derived::genome_t genome(Derived::inst_lib_t::DefaultInstLib());
    emp::Random random(55);
    for(size_t i = 0; i < 10; ++i)
      genome.push_back(cpu_init.GetRandomInst(random));
    CHECK(genome.size() == 10);
    // Create VirtualCPU using genome-only constructor
    Derived cpu(genome);
    // Ensure cpu has a two copies of genome - one to keep and one to work with
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 10);
    for(size_t i = 0; i < 10; ++i){
      CHECK(cpu.genome[i] == genome[i]);
      CHECK(cpu.genome_working[i] == genome[i]);
    }
    // VARIABLES -- should default other than genome / working genome
    {
      CHECK(cpu.stacks.size() == cpu.NUM_STACKS); // NUM_STACKS is actually used
      // All stacks are initially empty
      for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
        CHECK(cpu.stacks[idx].empty());
      }
      CHECK(cpu.active_stack_idx == 0);   // Default to first stack
      CHECK(cpu.GetNumNops() == 3);       // Default instruction set has 3 nops
      CHECK(cpu.nop_id_set.size() == 3);  // All three default nops present in set
      CHECK(emp::Has(cpu.nop_id_set, 0)); // NopA in set
      CHECK(emp::Has(cpu.nop_id_set, 1)); // NopB in set
      CHECK(emp::Has(cpu.nop_id_set, 2)); // NopC in set
      CHECK(cpu.GetNumRegs() == 3); // 3 nops in instruction set forces cpu to have 3 registers 
      // All registers start at their index value 
      for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
        CHECK(cpu.regs[idx] == idx);
      }
      CHECK(cpu.inputs.size() == 0);  // Start with no inputs
      CHECK(cpu.outputs.size() == 0); // Start with no outputs
      CHECK(cpu.inst_ptr == 0);   // All heads default to 0
      CHECK(cpu.read_head == 0);  // All heads default to 0
      CHECK(cpu.write_head == 0); // All heads default to 0
      CHECK(cpu.flow_head == 0);  // All heads default to 0
      CHECK(cpu.copied_inst_id_vec.size() == 0); // No instructions copied
      CHECK(cpu.label_idx_vec.size() == 0); // No labels in empty genome
      CHECK(cpu.nops_need_curated == true); // Nops are NOT curated at initialization 
      CHECK(cpu.expanded_nop_args == false);// Nop arguments are NOT expanded by default 
      CHECK(cpu.are_nops_counted == true);  // Defaults to false but flipped during initialization
      CHECK(cpu.are_regs_expanded == true); // Defaults to false but flipped during initialization
    }
  }
  { // Copy constructor
    // Create a CPU and change all possible variables 
    Derived cpu_init = CreateSeedCPU();
    // Create VirtualCPU using copy constructor
    Derived cpu(cpu_init);
    // Ensure genome and working genome were copied over
    CHECK(cpu.GetGenomeSize() == cpu_init.GetGenomeSize());
    for(size_t idx = 0; idx < cpu.genome.size(); ++idx)
      CHECK(cpu.genome[idx] == cpu_init.genome[idx]);
    CHECK(cpu.GetWorkingGenomeSize() == cpu_init.GetWorkingGenomeSize());
    for(size_t idx = 0; idx < cpu.genome_working.size(); ++idx)
      CHECK(cpu.genome_working[idx] == cpu.genome_working[idx]);
    // VARIABLES -- should 100% match those of the seed cpu 
    {
      for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
        CHECK(cpu.stacks[idx] == cpu_init.stacks[idx]);
      }
      CHECK(cpu.active_stack_idx == cpu_init.active_stack_idx);   
      CHECK(cpu.GetNumNops() == cpu_init.GetNumNops());       
      CHECK(cpu.nop_id_set.size() == cpu_init.nop_id_set.size());  
      for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
        CHECK(cpu.regs[idx] == cpu_init.regs[idx]);
      }
      CHECK(cpu.inputs.size() == cpu_init.inputs.size());
      CHECK(cpu.inputs[0] == cpu_init.inputs[0]);
      CHECK(cpu.outputs.size() == cpu_init.outputs.size()); 
      CHECK(cpu.outputs[0] == cpu_init.outputs[0]);
      CHECK(cpu.inst_ptr == cpu_init.inst_ptr);   
      CHECK(cpu.read_head == cpu_init.read_head);
      CHECK(cpu.write_head == cpu_init.write_head);
      CHECK(cpu.flow_head == cpu_init.flow_head);
      CHECK(cpu.copied_inst_id_vec.size() == cpu_init.copied_inst_id_vec.size());
      for(size_t idx = 0; idx < cpu.copied_inst_id_vec.size(); ++idx){
        CHECK(cpu.copied_inst_id_vec[idx] == cpu_init.copied_inst_id_vec[idx]);
      }
      CHECK(cpu.label_idx_vec.size() == cpu_init.label_idx_vec.size());
      for(size_t idx = 0; idx < cpu.label_idx_vec.size(); ++idx){
        CHECK(cpu.label_idx_vec[idx] == cpu_init.label_idx_vec[idx]);
      }
    }
  }
  { // Move constructor
    // Create a CPU and change all possible variables then repeat the process to invoke 
    // move constructor instead of copy constructor 
    Derived cpu_init = CreateSeedCPU();
    Derived cpu(CreateSeedCPU());
    // Ensure genome and working genome were copied over
    CHECK(cpu.GetGenomeSize() == cpu_init.GetGenomeSize());
    for(size_t idx = 0; idx < cpu.genome.size(); ++idx)
      CHECK(cpu.genome[idx] == cpu_init.genome[idx]);
    CHECK(cpu.GetWorkingGenomeSize() == cpu_init.GetWorkingGenomeSize());
    for(size_t idx = 0; idx < cpu.genome_working.size(); ++idx)
      CHECK(cpu.genome_working[idx] == cpu.genome_working[idx]);
    // VARIABLES -- should 100% match those of the seed cpu 
    {
      for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
        CHECK(cpu.stacks[idx] == cpu_init.stacks[idx]);
      }
      CHECK(cpu.active_stack_idx == cpu_init.active_stack_idx);   
      CHECK(cpu.GetNumNops() == cpu_init.GetNumNops());       
      CHECK(cpu.nop_id_set.size() == cpu_init.nop_id_set.size());  
      for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
        CHECK(cpu.regs[idx] == cpu_init.regs[idx]);
      }
      CHECK(cpu.inputs.size() == cpu_init.inputs.size());
      CHECK(cpu.inputs[0] == cpu_init.inputs[0]);
      CHECK(cpu.outputs.size() == cpu_init.outputs.size()); 
      CHECK(cpu.outputs[0] == cpu_init.outputs[0]);
      CHECK(cpu.inst_ptr == cpu_init.inst_ptr);   
      CHECK(cpu.read_head == cpu_init.read_head);
      CHECK(cpu.write_head == cpu_init.write_head);
      CHECK(cpu.flow_head == cpu_init.flow_head);
      CHECK(cpu.copied_inst_id_vec.size() == cpu_init.copied_inst_id_vec.size());
      for(size_t idx = 0; idx < cpu.copied_inst_id_vec.size(); ++idx){
        CHECK(cpu.copied_inst_id_vec[idx] == cpu_init.copied_inst_id_vec[idx]);
      }
      CHECK(cpu.label_idx_vec.size() == cpu_init.label_idx_vec.size());
      for(size_t idx = 0; idx < cpu.label_idx_vec.size(); ++idx){
        CHECK(cpu.label_idx_vec[idx] == cpu_init.label_idx_vec[idx]);
      }
    }
  }
  /*
   Constructors:
    [X] Default constructor gives us a default genome (empty)
    [X] Genome constructor sets genome/working genome 
    [X] Copy constructor functions as expected 
    [X] Move constructor functions as expected
    [X] Working genome is based on the actual genome
    [X] Variables are initialized as expected for each constructor (see above)
      [X] All defaults for default constructor
      [X] All but genome/working genome defaults for genome constructor
      [X] All variables copied over for copy constructor
      [X] All variables moved for move constructor
  */
}
TEST_CASE("VirtualCPU_Getters", "[Hardware]") {
  { // Default case
    Derived cpu;
    // Basic getters
    CHECK(cpu.GetNumRegs() == 3);
    CHECK(cpu.GetNumRegs() == cpu.regs.size()); 
    CHECK(cpu.GetNumNops() == 3);
    // Genome getters
    CHECK(cpu.GetGenomeSize() == 0);
    CHECK(cpu.GetWorkingGenomeSize() == 0);
    emp::Random random(1000);
    cpu.PushRandomInst(random, 10);
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 10);
    cpu.genome_working.push_back(cpu.GetRandomInst(random));
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 11);
    // Outputs
    CHECK(cpu.GetOutputs().size() == 0);
    for(size_t idx = 0; idx < 5; ++idx) cpu.outputs[idx] = idx * idx;
    CHECK(cpu.GetOutputs().size() == 5);
    for(size_t idx = 0; idx < 5; ++idx) CHECK(cpu.outputs[idx] == idx * idx);
    // Instruction library 
    CHECK(cpu.GetInstLib().Raw() == &Derived::inst_lib_t::DefaultInstLib()); 
  }
  {
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("NopE", Derived::inst_lib_t::Inst_NopC, 0, "No-operation E");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // Basic getters
    CHECK(cpu.GetNumRegs() == 5);
    CHECK(cpu.GetNumRegs() == cpu.regs.size()); 
    CHECK(cpu.GetNumNops() == 5);
  }
  /*
    [X] GetNumNops returns 
      [X] 3 by default
       [X] Something else if we stuff more nops in the instruction set
    [X] GetNumRegs returns 
      [X] 3 by default
      [X] Something else if we stuff more nops in the instruction set
    [X] GetGenomeSize returns original genome size
    [X] GetWorkingGenomeSize returns working genome size
    [X] GetOutputs returns ALL outputs 
    [X] GetInstLib returns correct pointer to instruction library
  */
}
TEST_CASE("VirtualCPU_Setters", "[Hardware]") {
  Derived cpu;
  CHECK(cpu.inputs.size() == 0);
  emp::vector<Derived::data_t> input_vec;
  input_vec.push_back(10);
  input_vec.push_back(22); 
  input_vec.push_back(50);
  cpu.SetInputs(input_vec);
  CHECK(cpu.inputs.size() == 3);
  CHECK(cpu.inputs[0] == 10);
  CHECK(cpu.inputs[1] == 22);
  CHECK(cpu.inputs[2] == 50);
  /*
    [X] SetInputs sets ALL inputs
  */
}
TEST_CASE("VirtualCPU_Genome_and_Instructions", "[Hardware]") {
  {
    Derived cpu;
    auto inst_lib_ptr = cpu.GetInstLib();
    emp::Random random(100);
    // GetDefaultInst 
    Derived::inst_t default_inst = cpu.GetDefaultInst();
    CHECK(default_inst.idx == 0);
    // GetRandomInst
    Derived::inst_t random_inst = cpu.GetRandomInst(random);
    for(size_t i = 0; (i < 20) && (random_inst.idx == 0); ++i) cpu.GetRandomInst(random);
    CHECK(random_inst.idx != 0);
    CHECK(random_inst.idx < inst_lib_ptr->GetSize());
    // PushInst(idx)
    CHECK(cpu.GetGenomeSize() == 0);
    CHECK(cpu.GetWorkingGenomeSize() == 0);
    cpu.CurateNops();
    CHECK(!cpu.nops_need_curated);
    cpu.PushInst(0); // Index 0 -> NopA
    CHECK(cpu.GetGenomeSize() == 1);
    CHECK(cpu.GetWorkingGenomeSize() == 1);
    CHECK(cpu.genome[0].idx == 0);
    CHECK(cpu.genome_working[0].idx == 0);
    CHECK(cpu.nops_need_curated);
    // PushInst(name)
    cpu.CurateNops();
    cpu.PushInst("NopB"); // NopB -> Index 1 
    CHECK(cpu.GetGenomeSize() == 2);
    CHECK(cpu.GetWorkingGenomeSize() == 2);
    CHECK(cpu.genome[1].idx == 1);
    CHECK(cpu.genome_working[1].idx == 1);
    CHECK(cpu.nops_need_curated);
    // PushInst(inst)
    cpu.CurateNops();
    cpu.PushInst(default_inst); // NopA -> Index 0 
    CHECK(cpu.GetGenomeSize() == 3);
    CHECK(cpu.GetWorkingGenomeSize() == 3);
    CHECK(cpu.genome[2].idx == 0);
    CHECK(cpu.genome_working[2].idx == 0);
    CHECK(cpu.nops_need_curated);
    // PushInst(inst, count)
    cpu.CurateNops();
    cpu.PushInst(default_inst, 5); // NopA -> Index 0 
    CHECK(cpu.GetGenomeSize() == 8);
    CHECK(cpu.GetWorkingGenomeSize() == 8);
    for(size_t i = 3; i < 8; ++i){
      CHECK(cpu.genome[i].idx == 0);
      CHECK(cpu.genome_working[i].idx == 0);
    }
    CHECK(cpu.nops_need_curated);
    // PushDefaultInst()
    cpu.CurateNops();
    cpu.PushDefaultInst(); // NopA -> Index 0 
    CHECK(cpu.GetGenomeSize() == 9);
    CHECK(cpu.GetWorkingGenomeSize() == 9);
    CHECK(cpu.genome[8].idx == 0);
    CHECK(cpu.nops_need_curated);
    // PushDefaultInst(count)
    cpu.CurateNops();
    cpu.PushDefaultInst(11); // NopA -> Index 0 
    CHECK(cpu.GetGenomeSize() == 20);
    CHECK(cpu.GetWorkingGenomeSize() == 20);
    for(size_t i = 9; i < 19; ++i){
      CHECK(cpu.genome[i].idx == 0);
      CHECK(cpu.genome_working[i].idx == 0);
    }
    CHECK(cpu.nops_need_curated);
    // SetInst(pos, inst)
    cpu.CurateNops();
    Derived::inst_t third_inst = Derived::inst_t(2); // NopC
    cpu.SetInst(0, third_inst);
    CHECK(cpu.genome[0].idx == 2);
    CHECK(cpu.genome_working[0].idx == 2);
    CHECK(cpu.GetGenomeSize() == 20);
    CHECK(cpu.GetWorkingGenomeSize() == 20);
    CHECK(cpu.nops_need_curated);
    // RandomizeInst(pos, rand)
    cpu.CurateNops();
    for(size_t i = 0; (i < 20) && (cpu.genome[0] == 2); ++i) cpu.RandomizeInst(0,random);
    CHECK(cpu.genome[0].idx != 2);
    CHECK(cpu.genome_working[0].idx != 2);
    CHECK(cpu.GetGenomeSize() == 20);
    CHECK(cpu.GetWorkingGenomeSize() == 20);
    CHECK(cpu.nops_need_curated);
    // PushRandomInst(rand)
    cpu.CurateNops();
    cpu.PushRandomInst(random);
    CHECK(cpu.GetGenomeSize() == 21);
    CHECK(cpu.GetWorkingGenomeSize() == 21);
    CHECK(cpu.nops_need_curated);
    // PushRandomInst(rand, count)
    cpu.CurateNops();
    cpu.PushRandomInst(random, 9);
    CHECK(cpu.GetGenomeSize() == 30);
    CHECK(cpu.GetWorkingGenomeSize() == 30);
    CHECK(cpu.nops_need_curated);
  }
  { // Load
    Derived cpu;
    CHECK(cpu.GetGenomeSize() == 0);
    CHECK(cpu.GetWorkingGenomeSize() == 0);
    cpu.CurateNops();
    CHECK(!cpu.nops_need_curated);
    std::stringstream sstr;
    sstr << "NopA\nAdd\nSub\nNopB\n"; 
    cpu.Load(sstr);
    CHECK(cpu.GetGenomeSize() == 4);
    CHECK(cpu.GetWorkingGenomeSize() == 4);
    CHECK(cpu.genome[0].idx == 0);
    CHECK(cpu.genome_working[0].idx == 0);
    CHECK(cpu.genome[3].idx == 1);
    CHECK(cpu.genome_working[3].idx == 1);
    cpu.Load("./ancestor_default.org"); // Load default avida ancestor from file
    CHECK(cpu.GetGenomeSize() == 50); // Should reset old genome
    CHECK(cpu.GetWorkingGenomeSize() == 50);
    CHECK(cpu.genome[2].idx == 2);
    CHECK(cpu.genome_working[2].idx == 2);
    CHECK(cpu.genome[49].idx == 1);
    CHECK(cpu.genome_working[49].idx == 1);
  }
  /*
    [X] PushInst adds the instruction to the end of the genome AND working genome
      [X] Update labels?
      [X] By index 
      [X] By name
      [X] By copy
      [X] By copy (multiple times)
    [X] PushDefaultInst pushes the first instruction in the library
      [X] Update labels?
    [X] GetRandomInst returns a random instruction within the instruction library
    [X] SetInst overwrites an instruction in the genome/working genome
      [X] Recalcuates labels/nops?
    [X] RandomizeInst calls SetInst on the genome position, but with a random instruction
    [X] Load 
      [X] Loads genome from a stream
      [X] That stream can be a file
  */
}
TEST_CASE("VirtualCPU_Head_Manipulation", "[Hardware]") {
  { // Instruction pointer 
    Derived cpu;
    emp::Random random(10);
    cpu.PushRandomInst(random, 10);
    CHECK(cpu.inst_ptr == 0);
    cpu.AdvanceIP();
    CHECK(cpu.inst_ptr == 1);
    cpu.AdvanceIP(3);
    CHECK(cpu.inst_ptr == 4);
    cpu.AdvanceIP(11);
    CHECK(cpu.inst_ptr == 5);
    cpu.ResetIP();
    CHECK(cpu.inst_ptr == 0);
    cpu.SetIP(7);
    CHECK(cpu.inst_ptr == 7);
    cpu.SetIP(18);
    CHECK(cpu.inst_ptr == 8);
    size_t idx = 0;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.inst_ptr == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.inst_ptr == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.inst_ptr == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.inst_ptr == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.inst_ptr == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.inst_ptr == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.inst_ptr == 8);
    idx = 16;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.inst_ptr == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.inst_ptr == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.inst_ptr == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.inst_ptr == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.inst_ptr == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.inst_ptr == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.inst_ptr == 8);
  }
  { // Read head 
    Derived cpu;
    emp::Random random(10);
    cpu.PushRandomInst(random, 10);
    CHECK(cpu.read_head == 0);
    cpu.AdvanceRH();
    CHECK(cpu.read_head == 1);
    cpu.AdvanceRH(3);
    CHECK(cpu.read_head == 4);
    cpu.AdvanceRH(11);
    CHECK(cpu.read_head == 5);
    cpu.ResetRH();
    CHECK(cpu.read_head == 0);
    cpu.SetRH(7);
    CHECK(cpu.read_head == 7);
    cpu.SetRH(18);
    CHECK(cpu.read_head == 8);
    size_t idx = 1;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.read_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.read_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.read_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.read_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.read_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.read_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.read_head == 8);
    idx = 17;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.read_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.read_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.read_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.read_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.read_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.read_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.read_head == 8);
  }
  { // Write head 
    Derived cpu;
    emp::Random random(10);
    cpu.PushRandomInst(random, 10);
    CHECK(cpu.write_head == 0);
    cpu.AdvanceWH();
    CHECK(cpu.write_head == 1);
    cpu.AdvanceWH(3);
    CHECK(cpu.write_head == 4);
    cpu.AdvanceWH(11);
    CHECK(cpu.write_head == 5);
    cpu.ResetWH();
    CHECK(cpu.write_head == 0);
    cpu.SetWH(7);
    CHECK(cpu.write_head == 7);
    cpu.SetWH(18);
    CHECK(cpu.write_head == 8);
    size_t idx = 2;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.write_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.write_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.write_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.write_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.write_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.write_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.write_head == 8);
    idx = 18;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.write_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.write_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.write_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.write_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.write_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.write_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.write_head == 8);
  }
  { // Flow head 
    Derived cpu;
    emp::Random random(10);
    cpu.PushRandomInst(random, 10);
    CHECK(cpu.flow_head == 0);
    cpu.AdvanceFH();
    CHECK(cpu.flow_head == 1);
    cpu.AdvanceFH(3);
    CHECK(cpu.flow_head == 4);
    cpu.AdvanceFH(11);
    CHECK(cpu.flow_head == 5);
    cpu.ResetFH();
    CHECK(cpu.flow_head == 0);
    cpu.SetFH(7);
    CHECK(cpu.flow_head == 7);
    cpu.SetFH(18);
    CHECK(cpu.flow_head == 8);
    size_t idx = 3;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.flow_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.flow_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.flow_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.flow_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.flow_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.flow_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.flow_head == 8);
    idx = 19;
    cpu.ResetModdedHead(idx); 
    CHECK(cpu.flow_head == 0);
    cpu.AdvanceModdedHead(idx); 
    CHECK(cpu.flow_head == 1);
    cpu.AdvanceModdedHead(idx, 3);
    CHECK(cpu.flow_head == 4);
    cpu.AdvanceModdedHead(idx, 11);
    CHECK(cpu.flow_head == 5);
    cpu.ResetModdedHead(idx);
    CHECK(cpu.flow_head == 0);
    cpu.SetModdedHead(idx, 7);
    CHECK(cpu.flow_head == 7);
    cpu.SetModdedHead(idx, 18);
    CHECK(cpu.flow_head == 8);
  }
  /*
    [X] ResetIP resets IP to 0
    [X] AdvanceIP moves IP forward and wraps around genome end if needed
    [X] SetIP assigns IP to certain location and wraps around genome end if needed
    [X] ResetRH resets RH to 0
    [X] AdvanceRH moves RH forward and wraps around genome end if needed
    [X] ResetRH resets RH to 0
    [X] AdvanceWH moves WH forward and wraps around genome end if needed
    [X] ResetWH resets WH to 0
    [X] AdvanceWH moves WH forward and wraps around genome end if needed
    [X] ResetFH resets FH to 0
    [X] AdvanceFH moves FH forward and wraps around genome end if needed
    [X] SetFH assigns FH to certain location and wraps around genome end if needed
    [X] ResetModdedHead resets head to 0
    [X] AdvanceModdedHead moves head forward and wraps around genome end if needed
    [X] SetModdedHead assigns head to certain location and wraps around genome end if needed
  */
}
TEST_CASE("VirtualCPU_Hardware_Manipulation", "[Hardware]") {
  { // ResetHardware
    // Create cpu with some variables set ...
    Derived cpu = CreateSeedCPU();
    cpu.copied_inst_id_vec.push_back(10);
    cpu.stacks[0].push_back(5);
    cpu.genome_working.push_back(Derived::inst_t(1));
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 11);
    cpu.ResetHardware();
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 11);
    { // VARIABLES -- should default
        CHECK(cpu.active_stack_idx == 0);   // Default to first stack
        CHECK(cpu.GetNumNops() == 3);       // Default instruction set has 3 nops
        CHECK(emp::Has(cpu.nop_id_set, 0)); // NopA in set
        CHECK(emp::Has(cpu.nop_id_set, 1)); // NopB in set
        CHECK(emp::Has(cpu.nop_id_set, 2)); // NopC in set
        CHECK(cpu.GetNumRegs() == 3); // 3 nops in instruction set forces cpu to have 3 registers 
        // All registers start at their index value 
        for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
          CHECK(cpu.regs[idx] == idx);
        }
        // All stacks are empty 
        for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
          CHECK(cpu.stacks[idx].empty());
        }
        CHECK(cpu.inputs.size() == 0);  // Start with no inputs
        CHECK(cpu.outputs.size() == 0); // Start with no outputs
        CHECK(cpu.inst_ptr == 0);   // All heads default to 0
        CHECK(cpu.read_head == 0);  // All heads default to 0
        CHECK(cpu.write_head == 0); // All heads default to 0
        CHECK(cpu.flow_head == 0);  // All heads default to 0
        CHECK(cpu.copied_inst_id_vec.size() == 0); // No instructions copied
    }
  }
  { // Reset
    // Create cpu with some variables set ...
    Derived cpu = CreateSeedCPU();
    cpu.copied_inst_id_vec.push_back(10);
    cpu.stacks[0].push_back(5);
    cpu.genome_working.push_back(Derived::inst_t(1));
    CHECK(cpu.GetGenomeSize() == 10);
    CHECK(cpu.GetWorkingGenomeSize() == 11);
    cpu.Reset();
    CHECK(cpu.GetGenomeSize() == 0);
    CHECK(cpu.GetWorkingGenomeSize() == 0);
    { // VARIABLES -- should default
        CHECK(cpu.active_stack_idx == 0);   // Default to first stack
        CHECK(cpu.GetNumNops() == 3);       // Default instruction set has 3 nops
        CHECK(emp::Has(cpu.nop_id_set, 0)); // NopA in set
        CHECK(emp::Has(cpu.nop_id_set, 1)); // NopB in set
        CHECK(emp::Has(cpu.nop_id_set, 2)); // NopC in set
        CHECK(cpu.GetNumRegs() == 3); // 3 nops in instruction set forces cpu to have 3 registers 
        // All registers start at their index value 
        for(size_t idx = 0; idx < cpu.GetNumRegs(); ++idx){
          CHECK(cpu.regs[idx] == idx);
        }
        // All stacks are empty 
        for(size_t idx = 0; idx < cpu.stacks.size(); ++idx){
          CHECK(cpu.stacks[idx].empty());
        }
        CHECK(cpu.inputs.size() == 0);  // Start with no inputs
        CHECK(cpu.outputs.size() == 0); // Start with no outputs
        CHECK(cpu.inst_ptr == 0);   // All heads default to 0
        CHECK(cpu.read_head == 0);  // All heads default to 0
        CHECK(cpu.write_head == 0); // All heads default to 0
        CHECK(cpu.flow_head == 0);  // All heads default to 0
        CHECK(cpu.copied_inst_id_vec.size() == 0); // No instructions copied
        CHECK(cpu.label_idx_vec.size() == 0); // No labels in empty genome
    }
  }
  { // CountNops and ExpandRegisters
    // Everything defaults to 3
    Derived cpu_default;
    CHECK(cpu_default.GetNumNops() == 3);
    CHECK(cpu_default.regs.size() == 3);
    CHECK(cpu_default.GetNumRegs() == 3);
    
    // Add NopD and NopE, which bumps us to 5 nops and registers
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("NopE", Derived::inst_lib_t::Inst_NopC, 0, "No-operation E");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu_more_nops(genome);
    CHECK(cpu_more_nops.GetNumNops() == 5);
    CHECK(cpu_more_nops.regs.size() == 5);
    CHECK(cpu_more_nops.GetNumRegs() == 5);
    
    // Add NopE but not NopD. Since they are disjoint we have still 3 nops and registers
    Derived::inst_lib_t inst_lib_2;
    inst_lib_2.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib_2.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib_2.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib_2.AddInst("NopE", Derived::inst_lib_t::Inst_NopC, 0, "No-operation E");
    Derived::genome_t genome_2 = Derived::genome_t(inst_lib_2);
    Derived cpu_bad_nops(genome_2);
    CHECK(cpu_bad_nops.GetNumNops() == 3);
    CHECK(cpu_bad_nops.regs.size() == 3);
    CHECK(cpu_bad_nops.GetNumRegs() == 3);
  }
  { // CurateNops
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    cpu.PushInst("NopA");  // 0 
    cpu.PushInst("NopB");  // 1
    cpu.PushInst("NopC");  // 2
    cpu.PushInst("NopD");  // 3
    cpu.PushInst("Label"); // 4
    cpu.PushInst("NopD");  // 5
    cpu.PushInst("NopB");  // 6
    cpu.PushInst("Add");   // 7 
    cpu.PushInst("NopC");  // 8
    cpu.PushInst("Label"); // 9
    cpu.PushInst("NopD");  // 10
    CHECK(cpu.nops_need_curated);
    cpu.CurateNops();
    CHECK(!cpu.nops_need_curated);
    CHECK(cpu.label_idx_vec.size() == 2);
    CHECK(cpu.label_idx_vec[0] == 4);
    CHECK(cpu.label_idx_vec[1] == 9);
    { // Nop vec checks 
      { // 0
        CHECK(cpu.genome_working[0].nop_vec.size() == 3);
        CHECK(cpu.genome_working[0].nop_vec[0] == 1);
        CHECK(cpu.genome_working[0].nop_vec[1] == 2);
        CHECK(cpu.genome_working[0].nop_vec[2] == 3);
      }
      { // 1
        CHECK(cpu.genome_working[1].nop_vec.size() == 2);
        CHECK(cpu.genome_working[1].nop_vec[0] == 2);
        CHECK(cpu.genome_working[1].nop_vec[1] == 3);
      }
      { // 2
        CHECK(cpu.genome_working[2].nop_vec.size() == 1);
        CHECK(cpu.genome_working[2].nop_vec[0] == 3);
      }
      { // 3
        CHECK(cpu.genome_working[3].nop_vec.size() == 0);
      }
      { // 4
        CHECK(cpu.genome_working[4].nop_vec.size() == 2);
        CHECK(cpu.genome_working[4].nop_vec[0] == 3);
        CHECK(cpu.genome_working[4].nop_vec[1] == 1);
      }
      { // 5
        CHECK(cpu.genome_working[5].nop_vec.size() == 1);
        CHECK(cpu.genome_working[5].nop_vec[0] == 1);
      }
      { // 6
        CHECK(cpu.genome_working[6].nop_vec.size() == 0);
      }
      { // 7
        CHECK(cpu.genome_working[7].nop_vec.size() == 1);
        CHECK(cpu.genome_working[7].nop_vec[0] == 2);
      }
      { // 8
        CHECK(cpu.genome_working[8].nop_vec.size() == 0);
      }
      { // 9
        CHECK(cpu.genome_working[9].nop_vec.size() == 5);
        CHECK(cpu.genome_working[9].nop_vec[0] == 3);
        CHECK(cpu.genome_working[9].nop_vec[1] == 0);
        CHECK(cpu.genome_working[9].nop_vec[2] == 1);
        CHECK(cpu.genome_working[9].nop_vec[3] == 2);
        CHECK(cpu.genome_working[9].nop_vec[4] == 3);
      }
      { // 10 
        CHECK(cpu.genome_working[10].nop_vec.size() == 4);
        CHECK(cpu.genome_working[10].nop_vec[0] == 0);
        CHECK(cpu.genome_working[10].nop_vec[1] == 1);
        CHECK(cpu.genome_working[10].nop_vec[2] == 2);
        CHECK(cpu.genome_working[10].nop_vec[3] == 3);
      }
    }
  }
  /*
    [X] ResetHardware resets:
      [X] Registers
      [X] Heads
      [X] Stacks
      [X] Inputs
      [X] Outputs
      [X] Working genome ???
      [X] Copied instructions
    [X] Reset
      [X] ResetHardware
      [X] Reset genome
      [X] Reset working genome
      [X] Labels
    [X] CurateNops
      [X] Counts nops if needed
      [X] Finds all labels
      [X] Add nops to preceeding instructions nop_vec
      [X] Wraps?
      [X] Sets boolean flag to false
    [X] CountNops
      [X] Calculates the number of nops
      [X] Maps nop ids to indices
      [X] Maps nop indices to ids
    [X] ExpandRegisters
      [X] Sets num_regs variable
      [X] Resizes register vector
  */
}
TEST_CASE("VirtualCPU_Nop_Methods", "[Hardware]") {
  { // GetComplementIdx
    { // Standard instruction set
      Derived cpu;
      CHECK(cpu.GetComplementNop(0) == 1); // A->B
      CHECK(cpu.GetComplementNop(1) == 2); // B->C
      CHECK(cpu.GetComplementNop(2) == 0); // C->A
    }
    { // Extended instruction set
      Derived::inst_lib_t inst_lib;
      inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
      inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
      inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
      inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
      inst_lib.AddInst("NopE", Derived::inst_lib_t::Inst_NopC, 0, "No-operation E");
      Derived::genome_t genome = Derived::genome_t(inst_lib);
      Derived cpu(genome);
      CHECK(cpu.GetComplementNop(0) == 1); // A->B
      CHECK(cpu.GetComplementNop(1) == 2); // B->C
      CHECK(cpu.GetComplementNop(2) == 3); // C->D
      CHECK(cpu.GetComplementNop(3) == 4); // D->E
      CHECK(cpu.GetComplementNop(4) == 0); // E->A
    }
  }
  { // GetComplementLabel
    { // Standard instruction set
      Derived cpu;
      emp::vector<size_t> v {2, 1, 0, 0, 2, 1};
      emp::vector<size_t> res = cpu.GetComplementNopSequence(v);
      CHECK(res[0] == 0); // C->A
      CHECK(res[1] == 2); // B->C
      CHECK(res[2] == 1); // A->B
      CHECK(res[3] == 1); // A->B
      CHECK(res[4] == 0); // C->A
      CHECK(res[5] == 2); // B->C
    }
    { // Extended instruction set
      Derived::inst_lib_t inst_lib;
      inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
      inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
      inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
      inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
      inst_lib.AddInst("NopE", Derived::inst_lib_t::Inst_NopC, 0, "No-operation E");
      Derived::genome_t genome = Derived::genome_t(inst_lib);
      Derived cpu(genome);
      emp::vector<size_t> v {2, 1, 0, 0, 2, 1, 3, 4, 3, 1, 4};
      emp::vector<size_t> res = cpu.GetComplementNopSequence(v);
      CHECK(res[0] == 3); // C->D
      CHECK(res[1] == 2); // B->C
      CHECK(res[2] == 1); // A->B
      CHECK(res[3] == 1); // A->B
      CHECK(res[4] == 3); // C->D
      CHECK(res[5] == 2); // B->C
      CHECK(res[6] == 4); // D->E
      CHECK(res[7] == 0); // E->A
      CHECK(res[8] == 4); // D->E
      CHECK(res[9] == 2); // B->C
      CHECK(res[10] ==0); // E->A
    }
  }
  { // CompareNopSequences
    // Note: here we only use three nops. However, this code is dealing solely with size_ts so
    //    it should generalize (there are no checks on if the nop is invalid)
    Derived cpu;
    // Perfect match
    emp::vector<size_t> v_1_a {0, 1, 2, 1, 0 };
    emp::vector<size_t> v_1_b {0, 1, 2, 1, 0 };
    CHECK(cpu.CompareNopSequences(v_1_a, v_1_b));
    // Second vector can be longer than first
    emp::vector<size_t> v_2_a {0, 1, 2, 1, 0 };
    emp::vector<size_t> v_2_b {0, 1, 2, 1, 0, 1, 2 };
    CHECK(cpu.CompareNopSequences(v_2_a, v_2_b));
    // First vector CANNOT be longer than second
    emp::vector<size_t> v_3_a {0, 1, 2, 1, 0, 1, 2 };
    emp::vector<size_t> v_3_b {0, 1, 2, 1, 0 };
    CHECK(!cpu.CompareNopSequences(v_3_a, v_3_b));
    // First vector CANNOT be empty
    emp::vector<size_t> v_4_a { };
    emp::vector<size_t> v_4_b {0, 1, 2, 1, 0 };
    CHECK(!cpu.CompareNopSequences(v_4_a, v_4_b));
    // Second vector CANNOT be empty
    emp::vector<size_t> v_5_a {0, 1, 2};
    emp::vector<size_t> v_5_b { };
    CHECK(!cpu.CompareNopSequences(v_5_a, v_5_b));
    // Both vectors CANNOT be empty
    emp::vector<size_t> v_6_a { };
    emp::vector<size_t> v_6_b { };
    CHECK(!cpu.CompareNopSequences(v_6_a, v_6_b));
    // Mismatch -> return false
    emp::vector<size_t> v_7_a {0, 1, 2 };
    emp::vector<size_t> v_7_b {0, 2, 2 };
    CHECK(!cpu.CompareNopSequences(v_7_a, v_7_b));
    // Match occurs after mismatch -> still fail
    emp::vector<size_t> v_8_a {0, 1, 2 };
    emp::vector<size_t> v_8_b {0, 2, 2, 0, 1, 2};
    CHECK(!cpu.CompareNopSequences(v_8_a, v_8_b));
  }
  { // CheckIfLastCopied
    Derived cpu;
    cpu.copied_inst_id_vec = {0, 1, 2};
    // True
    CHECK(cpu.CheckIfLastCopied({0, 1, 2}));
    CHECK(cpu.CheckIfLastCopied({1, 2}));
    CHECK(cpu.CheckIfLastCopied({2}));
    CHECK(cpu.CheckIfLastCopied({2}));
    // False
    CHECK(!cpu.CheckIfLastCopied({1})); // Mismatch
    CHECK(!cpu.CheckIfLastCopied({0, 2, 2})); // Mismatch with correct size
    CHECK(!cpu.CheckIfLastCopied({0, 1, 2, 0})); // Too long
    CHECK(!cpu.CheckIfLastCopied({})); // Empty 
  }
  { // FindLabel_Reverse(start_local)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x L A B x L C D x A B  x  C  D  x  L  A  B  x  D  A
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
    std::stringstream sstr;
    sstr << "Add\nLabel\nNopA\nNopB\nAdd\nLabel\nNopC\nNopD\n"
            "Add\nNopA\nNopB\nAdd\nNopC\nNopD\nAdd\nLabel\nNopA\nNopB\nAdd\nNopD\nNopA";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Does start_local actually factor in?
    cpu.inst_ptr = 8;
    CHECK(cpu.FindLabel_Reverse(true) == 1);
    CHECK(cpu.FindLabel_Reverse(false) == 15);
    // If we have only one label with that nop sequence, it returns regardless of start_local
    // Also, ensure Nops after NopC work too
    cpu.inst_ptr = 11;
    CHECK(cpu.FindLabel_Reverse(true) == 5);
    CHECK(cpu.FindLabel_Reverse(false) == 5);
    // If instruction pointer is on the only label with that sequence, return inst_ptr
    cpu.inst_ptr = 5;
    CHECK(cpu.FindLabel_Reverse(true) == 5);
    CHECK(cpu.FindLabel_Reverse(false) == 5);
    // If instruction pointer is on a label and another match exists, 
    // Return it if start_local = true
    // Return depending on position if start_local = false
    cpu.inst_ptr = 15;
    CHECK(cpu.FindLabel_Reverse(true) == 1);
    CHECK(cpu.FindLabel_Reverse(false) == 15);
    // If no matching label found, return instruction pointer
    cpu.inst_ptr = 18;
    CHECK(cpu.FindLabel_Reverse(true) == 18);
    CHECK(cpu.FindLabel_Reverse(false) == 18);
  }
  { // FindLabel(start_local, reverse = false)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x L A B x L C D x A B  x  C  D  x  L  A  B  x  D  A
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20
    std::stringstream sstr;
    sstr << "Add\nLabel\nNopA\nNopB\nAdd\nLabel\nNopC\nNopD\n"
            "Add\nNopA\nNopB\nAdd\nNopC\nNopD\nAdd\nLabel\nNopA\nNopB\nAdd\nNopD\nNopA";
    cpu.Load(sstr);
    cpu.CurateNops();
    { // Use FindLabel_Reverse tests if reverse = true
      // Does start_local actually factor in?
      cpu.inst_ptr = 8;
      CHECK(cpu.FindLabel(true, true) == 1);
      CHECK(cpu.FindLabel(false, true) == 15);
      // If we have only one label with that sequence, it returns regardless of start_local
      // Also, ensure Nops after NopC work too
      cpu.inst_ptr = 11;
      CHECK(cpu.FindLabel(true, true) == 5);
      CHECK(cpu.FindLabel(false, true) == 5);
      // If instruction pointer is on the only label with that sequence, return inst_ptr
      cpu.inst_ptr = 5;
      CHECK(cpu.FindLabel(true, true) == 5);
      CHECK(cpu.FindLabel(false, true) == 5);
      // If instruction pointer is on a label and another match exists, 
      // Return it if start_local = true
      // Return depending on position if start_local = false
      cpu.inst_ptr = 15;
      CHECK(cpu.FindLabel(true, true) == 1);
      CHECK(cpu.FindLabel(false, true) == 15);
      // If no matching label found, return instruction pointer
      cpu.inst_ptr = 18;
      CHECK(cpu.FindLabel(true, true) == 18);
      CHECK(cpu.FindLabel(false, true) == 18);
    }
    { // reverse = false
      // Does start_local actually factor in?
      cpu.inst_ptr = 8;
      CHECK(cpu.FindLabel(true) == 15);
      CHECK(cpu.FindLabel(false) == 1);
      // If we have only one label with that sequence, it returns regardless of start_local
      // Also, ensure Nops after NopC work too
      cpu.inst_ptr = 11;
      CHECK(cpu.FindLabel(true) == 5);
      CHECK(cpu.FindLabel(false) == 5);
      // If instruction pointer is on the only label with that sequence, return inst_ptr
      cpu.inst_ptr = 5;
      CHECK(cpu.FindLabel(true) == 5);
      CHECK(cpu.FindLabel(false) == 5);
      // If instruction pointer is on a label and another match exists, 
      // Return it if start_local = true
      // Return depending on position if start_local = false
      cpu.inst_ptr = 15;
      CHECK(cpu.FindLabel(true) == 1);
      CHECK(cpu.FindLabel(false) == 1);
      // If no matching label found, return instruction pointer
      cpu.inst_ptr = 18;
      CHECK(cpu.FindLabel(true) == 18);
      CHECK(cpu.FindLabel(false) == 18);
    }
  }
  { //FindNopSequence_Reverse(search_vec, start_idx)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Keep instruction pointer at beginning of genome and use start_idx instead
    cpu.inst_ptr = 2; 
    // Ensure start_idx is used and search does not include the current instruction pointer
    CHECK(cpu.FindNopSequence_Reverse({0, 1}, (size_t)0) == 16);
    CHECK(cpu.FindNopSequence_Reverse({0, 1}, (size_t)6) == 0);
    CHECK(cpu.FindNopSequence_Reverse({0, 1}, (size_t)16) == 6);
    // If sequence only appears once, always return it (also check if NopD is valid)
    CHECK(cpu.FindNopSequence_Reverse({3, 3}, (size_t)0) == 13);
    CHECK(cpu.FindNopSequence_Reverse({3, 3}, (size_t)12) == 13);
    CHECK(cpu.FindNopSequence_Reverse({3, 3}, (size_t)13) == 13);
    CHECK(cpu.FindNopSequence_Reverse({3, 3}, (size_t)14) == 13);
    CHECK(cpu.FindNopSequence_Reverse({3, 3}, (size_t)18) == 13);
    // Found sequence can have extra nops 
    CHECK(cpu.FindNopSequence_Reverse({1}, (size_t)0) == 17);
    CHECK(cpu.FindNopSequence_Reverse({1}, (size_t)9) == 7);
    CHECK(cpu.FindNopSequence_Reverse({1}, (size_t)17) == 9);
    // If pattern not found, return instruction pointer 
    CHECK(cpu.FindNopSequence_Reverse({1,1,1}, (size_t)0) == 2);
    CHECK(cpu.FindNopSequence_Reverse({1,1,1}, (size_t)9) == 2);
    CHECK(cpu.FindNopSequence_Reverse({1,1,1}, (size_t)17) == 2);
  }
  { //FindNopSequence_Reverse(search_vec, start_local)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Ensure result always matches the already-tested overload that uses start_idx 
    emp::vector<emp::vector<size_t>> test_vectors;
    test_vectors.push_back(emp::vector<size_t>({0,1}));
    test_vectors.push_back(emp::vector<size_t>({3,3}));
    test_vectors.push_back(emp::vector<size_t>({1}));
    test_vectors.push_back(emp::vector<size_t>({1,1,1}));
    for(emp::vector<size_t> v : test_vectors){
      for(size_t idx = 0; idx < cpu.GetGenomeSize(); ++idx){
        cpu.inst_ptr = idx;
        CHECK(cpu.FindNopSequence_Reverse(v, true) == 
            cpu.FindNopSequence_Reverse(v, (size_t)cpu.inst_ptr));
        CHECK(cpu.FindNopSequence_Reverse(v, false) == 
            cpu.FindNopSequence_Reverse(v, (size_t)0));
      }
    }
  }
  { //FindNopSequence_Reverse(start_local)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Ensure result always matches the already-tested overload that uses start_idx 
    emp::vector<emp::vector<size_t>> test_vectors;
    test_vectors.push_back(emp::vector<size_t>({0,1}));
    test_vectors.push_back(emp::vector<size_t>({3,3}));
    test_vectors.push_back(emp::vector<size_t>({1}));
    test_vectors.push_back(emp::vector<size_t>({1,1,1}));
    for(emp::vector<size_t> v : test_vectors){
      for(size_t idx = 0; idx < cpu.GetGenomeSize(); ++idx){
        cpu.inst_ptr = idx;
        CHECK(cpu.FindNopSequence_Reverse(true) == 
            cpu.FindNopSequence_Reverse(
                cpu.genome_working[cpu.inst_ptr].nop_vec, (size_t)cpu.inst_ptr
            )
        );
        CHECK(cpu.FindNopSequence_Reverse(false) == 
            cpu.FindNopSequence_Reverse(cpu.genome_working[cpu.inst_ptr].nop_vec, (size_t)0));
      }
    }
  }
  { //FindNopSequence(search_vec, start_idx, reverse = false)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Keep instruction pointer at beginning of genome and use start_idx instead
    cpu.inst_ptr = 15; 
    // Ensure start_idx is used and search does not include the current instruction pointer
    CHECK(cpu.FindNopSequence({0, 1}, (size_t)0) == 6);
    CHECK(cpu.FindNopSequence({0, 1}, (size_t)6) == 16);
    CHECK(cpu.FindNopSequence({0, 1}, (size_t)16) ==0);
    // If sequence only appears once, always return it (also check if NopD is valid)
    CHECK(cpu.FindNopSequence({3, 3}, (size_t)0) == 13);
    CHECK(cpu.FindNopSequence({3, 3}, (size_t)12) == 13);
    CHECK(cpu.FindNopSequence({3, 3}, (size_t)13) == 13);
    CHECK(cpu.FindNopSequence({3, 3}, (size_t)14) == 13);
    CHECK(cpu.FindNopSequence({3, 3}, (size_t)18) == 13);
    // Found sequence can have extra nops 
    CHECK(cpu.FindNopSequence(emp::vector<size_t>({1}), (size_t)0) == 1);
    CHECK(cpu.FindNopSequence(emp::vector<size_t>({1}), (size_t)9) == 17);
    CHECK(cpu.FindNopSequence(emp::vector<size_t>({1}), (size_t)17) == 1);
    // If pattern not found, return instruction pointer 
    CHECK(cpu.FindNopSequence({1,1,1}, (size_t)0) == cpu.inst_ptr);
    CHECK(cpu.FindNopSequence({1,1,1}, (size_t)9) == cpu.inst_ptr);
    CHECK(cpu.FindNopSequence({1,1,1}, (size_t)17) == cpu.inst_ptr);
    { // Reverse
      emp::vector<emp::vector<size_t>> test_vectors;
      test_vectors.push_back(emp::vector<size_t>({0,1}));
      test_vectors.push_back(emp::vector<size_t>({3,3}));
      test_vectors.push_back(emp::vector<size_t>({1}));
      test_vectors.push_back(emp::vector<size_t>({1,1,1}));
      for(emp::vector<size_t> v : test_vectors){
        for(size_t idx = 0; idx < cpu.GetGenomeSize(); ++idx){
          CHECK(cpu.FindNopSequence(v, idx, true) == cpu.FindNopSequence_Reverse(v, idx));
        }
      }
    }
  }
  { //FindNopSequence(search_vec, start_local, reverse = false)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    cpu.inst_ptr = 15; 
    emp::vector<emp::vector<size_t>> test_vectors;
    test_vectors.push_back(emp::vector<size_t>({0,1}));
    test_vectors.push_back(emp::vector<size_t>({3,3}));
    test_vectors.push_back(emp::vector<size_t>({1}));
    test_vectors.push_back(emp::vector<size_t>({1,1,1}));
    // Ensure output matches that of previously-tested overload
    for(emp::vector<size_t> v : test_vectors){
      for(size_t idx = 0; idx < cpu.GetGenomeSize(); ++idx){
        cpu.inst_ptr = idx;
        CHECK(cpu.FindNopSequence(v, true, false) == cpu.FindNopSequence(v, cpu.inst_ptr));
        CHECK(cpu.FindNopSequence(v, true, true) == cpu.FindNopSequence(v,cpu.inst_ptr,true));
        CHECK(cpu.FindNopSequence(v, false, false) == 
            cpu.FindNopSequence(v,cpu.GetWorkingGenomeSize() - 1));
        CHECK(cpu.FindNopSequence(v, false, true) == 
            cpu.FindNopSequence(v,cpu.GetWorkingGenomeSize() - 1,true));
      }
    }
  }
  { //FindNopSequence(start_local, reverse = false)
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopA\nAdd\nNopA\nNopB\nAdd\nNopB\nNopA\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    // Verify output matches previously tested overload
    for(size_t idx = 0; idx < cpu.GetGenomeSize(); ++idx){
      cpu.inst_ptr = idx;
      CHECK(cpu.FindNopSequence(true, false) == 
          cpu.FindNopSequence(cpu.genome_working[cpu.inst_ptr].nop_vec, cpu.inst_ptr));
      CHECK(cpu.FindNopSequence(true, true) == 
          cpu.FindNopSequence(cpu.genome_working[cpu.inst_ptr].nop_vec, cpu.inst_ptr, true));
      CHECK(cpu.FindNopSequence(false, false) == 
          cpu.FindNopSequence(
              cpu.genome_working[cpu.inst_ptr].nop_vec, cpu.GetWorkingGenomeSize() - 1
          )
      );
      CHECK(cpu.FindNopSequence(false, true) == 
          cpu.FindNopSequence(cpu.genome_working[cpu.inst_ptr].nop_vec, 
              cpu.GetWorkingGenomeSize() - 1,true
          )
     );
    }
  }
  /*
    [X] GetComplementIdx returns the complement of a single nop 
      [X] Standard nops
      [X] Expanded nops
    [X] GetComplementLabel returns the complemented sequences of nops (rename) 
      [X] Standard nops
      [X] Expanded nops
    [X] CompareSequences determines if two nop vectors are identical
      [X] Check length too!
    [X] CheckIfLastCopied only returns true if the nop vector was the last thing copied
    [X] FindLabel_Reverse(start_local){
    [X] FindLabel(start_local, reverse = false) 
    [X] FindNopSequence_Reverse(search_vec, start_idx) 
    [X] FindNopSequence_Reverse(search_vec, start_local) 
    [X] FindNopSequence_Reverse(start_local) 
    [X] FindNopSequence(search_vec, start_idx, reverse = false) 
    [X] FindNopSequence(search_vec, start_local, reverse = false) 
    [X] FindNopSequence(start_local, reverse = false) 
  */
}
TEST_CASE("VirtualCPU_Stack_Methods", "[Hardware]") {
  Derived cpu;
  // Default stack
  CHECK(cpu.active_stack_idx == 0); 
  cpu.regs[0] = 1;
  cpu.regs[1] = 2;
  cpu.regs[2] = 3;
  cpu.StackPush(0);
  CHECK(cpu.stacks[0].size() == 1); 
  CHECK(cpu.stacks[0][0] == 1); 
  cpu.StackPush(2);
  CHECK(cpu.stacks[0].size() == 2); 
  CHECK(cpu.stacks[0][1] == 3); 
  cpu.StackPop(0);
  CHECK(cpu.stacks[0].size() == 1); 
  CHECK(cpu.regs[0] == 3); 
  // Swap stacks
  cpu.StackSwap();
  CHECK(cpu.active_stack_idx == 1); 
  cpu.regs[0] = 1;
  cpu.regs[1] = 2;
  cpu.regs[2] = 3;
  cpu.StackPush(0);
  CHECK(cpu.stacks[1].size() == 1); 
  CHECK(cpu.stacks[1][0] == 1); 
  cpu.StackPush(2);
  CHECK(cpu.stacks[1].size() == 2); 
  CHECK(cpu.stacks[1][1] == 3); 
  cpu.StackPop(0);
  CHECK(cpu.stacks[1].size() == 1); 
  CHECK(cpu.regs[0] == 3); 
  // Swap back
  cpu.StackSwap();
  CHECK(cpu.active_stack_idx == 0); 
  /*
    [X] StackPush pushes the register value onto the active stack
    [X] StackPop pops the top value of the active stack and stores it in a register
    [X] StackSwap actually swaps the active stack
  */
}
TEST_CASE("VirtualCPU_Processing_Methods", "[Hardware]") {
  // SingleProcess(verbose = true)
  Derived cpu;
  cpu.PushDefaultInst(10);
  CHECK(cpu.GetGenomeSize() == 10);
  CHECK(cpu.GetWorkingGenomeSize() == 10);
  CHECK(cpu.nops_need_curated);
  CHECK(cpu.inst_ptr == 0);
  cpu.SingleProcess(false);
  CHECK(cpu.inst_ptr == 1); // IP incremented
  CHECK(!cpu.nops_need_curated); // IP incremented
  cpu.SingleProcess(false);
  CHECK(cpu.inst_ptr == 2); // IP incremented
  // Process(count = 1, verbose = true)
  cpu.nops_need_curated = true;
  cpu.Process();
  CHECK(cpu.inst_ptr == 3); // IP incremented
  CHECK(!cpu.nops_need_curated); // IP incremented
  cpu.Process(4, false);
  CHECK(cpu.inst_ptr == 7); // IP incremented count times
  cpu.Process(5, false);
  CHECK(cpu.inst_ptr == 2); // IP wraps
  /*
    [X] SingleProcess
      [X] Do bookkeeping as needed based on flags
      [X] Process the next instruction
      [X] Auto-advance IP?
    [X] Process
      [X] Calls SingleProcess N times
  */
}
TEST_CASE("VirtualCPU_String_Methods", "[Hardware]") {
    Derived::inst_lib_t inst_lib;
    inst_lib.AddInst("NopA", Derived::inst_lib_t::Inst_NopA, 0, "No-operation A");
    inst_lib.AddInst("NopB", Derived::inst_lib_t::Inst_NopB, 0, "No-operation B");
    inst_lib.AddInst("NopC", Derived::inst_lib_t::Inst_NopC, 0, "No-operation C");
    inst_lib.AddInst("NopD", Derived::inst_lib_t::Inst_NopC, 0, "No-operation D");
    inst_lib.AddInst("Label", Derived::inst_lib_t::Inst_NopC, 0, "Fake label");
    inst_lib.AddInst("Add", Derived::inst_lib_t::Inst_NopC, 0, "Fake add");
    Derived::genome_t genome = Derived::genome_t(inst_lib);
    Derived cpu(genome);
    // x A B x B D x A B x B  D  C  x  D  D  L  A  B
    // 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18
    std::stringstream sstr;
    sstr << "Add\nNopA\nNopB\nAdd\nNopB\nNopD\nAdd\nNopA\nNopB\nAdd\nNopB\nNopD\nNopC\n"
            "Add\nNopD\nNopD\nLabel\nNopA\nNopB";
    cpu.Load(sstr);
    cpu.CurateNops();
    CHECK(cpu.GetGenomeString() == "[19]fabfbdfabfbdcfddeab");
    CHECK(cpu.GetWorkingGenomeString() == "[19]fabfbdfabfbdcfddeab");
    cpu.genome_working.resize(3, 0);
    CHECK(cpu.GetGenomeString() == "[19]fabfbdfabfbdcfddeab");
    CHECK(cpu.GetWorkingGenomeString() == "[3]fab");
    std::stringstream ostr;
    cpu.PrintDetails(ostr);
    std::cout << ostr.str();
    CHECK(ostr.str().size() > 0);
    emp::vector<std::string> string_vec;
    std::string output_str = ostr.str();
    emp::slice(ostr.str(), string_vec, '\n');
    CHECK(string_vec.size() == 5);
    CHECK(string_vec[0] == "IP: 0 RH: 0 WH: 0 FH: 0(nops: 4; regs: 4)");
    CHECK(string_vec[1] == "[0] 0");
    CHECK(string_vec[2] == "[1] 1");
    CHECK(string_vec[3] == "[2] 2");
    CHECK(string_vec[4] == "[3] 3\n"); // last line gets to keep its newline character
  /*
    [X] GetWorkingGenomeString returns a string representation of the working genome
    [X] GetGenomeString returns a string representation of the (non-working) genome
    [X] Print details dumps registers and such to the stream
  */
}
