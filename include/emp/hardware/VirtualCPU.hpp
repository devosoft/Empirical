/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2021.
 *
 *  @file  VirtualCPU.hpp
 *  @brief This is a simple virtual CPU that can be extended into multiple architectures
 *    e.g., Avidians, Push, SignalGP
 *
 *  @TODO
 *    - Figure out how to default instructions
 *    - Should PushInst update labels?
 *      - SetInst
 *    - Rename nop and label methods
 *    - Add docstrings
 *    - Curate nops should wrap
 *    
 */

#ifndef EMP_VIRTUAL_CPU_H
#define EMP_VIRTUAL_CPU_H

#include <fstream>
#include <iostream>
#include <map>

#include "../base/array.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../base/unordered_map.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../io/File.hpp"
#include "../math/Random.hpp"
#include "../tools/string_utils.hpp"

#include "VirtualCPU_InstLib.hpp"
#include "Genome.hpp"

namespace emp{
  template <typename DERIVED>
  class VirtualCPU{
    public:
      struct Instruction {
        size_t idx;
        size_t id;
        emp::vector<size_t> nop_vec;

        Instruction() = delete; 
        Instruction(size_t _idx, size_t _id=0, emp::vector<size_t> _nop_vec = {})
          : idx(_idx), id(_id), nop_vec(_nop_vec) { ; }
        Instruction(const Instruction &) = default;
        Instruction(Instruction &&) = default;

        Instruction & operator=(const Instruction &) = default;
        Instruction & operator=(Instruction &&) = default;
        bool operator<(const Instruction & in) const {
            return id < in.id;
        }
        bool operator==(const Instruction & in) const { return id == in.id; }
        bool operator!=(const Instruction & in) const { return !(*this == in); }
        bool operator>(const Instruction & in) const { return in < *this; }
        bool operator>=(const Instruction & in) const { return !(*this < in); }
        bool operator<=(const Instruction & in) const { return !(in < *this); }

        void Set(size_t _idx, size_t _id, emp::vector<size_t> _nop_vec = {})
          { idx = _idx; id = _id; nop_vec=_nop_vec;}
      };
      static constexpr size_t NUM_STACKS = 2;
      static constexpr size_t INST_ARGS = 0;
      
      using derived_t = DERIVED;
      using data_t = uint32_t;
      using inst_t = Instruction;
      using inst_lib_t = VirtualCPU_InstLib<derived_t, data_t, 0>;
      using genome_t = Genome<Instruction, inst_lib_t>;
      using nop_vec_t = emp::vector<size_t>;
      using stack_t = emp::vector<double>;
    
    protected:
      size_t num_regs = 0;
      size_t num_nops = 0;

    public:
      bool are_nops_counted = false;
      bool are_regs_expanded = false;
      emp::unordered_map<size_t, size_t> nop_id_to_idx_map;
      emp::unordered_map<size_t, size_t> nop_idx_to_id_map;
      
      emp::vector<data_t> regs;
      std::unordered_map<int, data_t> inputs;  // Map of all available inputs (position -> value)
      std::unordered_map<int, data_t> outputs; // Map of all outputs (position -> value)
      emp::array<stack_t, NUM_STACKS> stacks;
      size_t active_stack_idx = 0;

      size_t inst_ptr;
      size_t flow_head;
      size_t read_head;
      size_t write_head;

      genome_t genome;
      genome_t genome_working;
      emp::vector<size_t> copied_inst_id_vec;
      emp::vector<size_t> label_idx_vec;

      bool needs_nops_curated = true;
      bool expanded_nop_args = false;
    

      //// CONSTRUCTORS / DESTRUCTOR
      VirtualCPU(const genome_t & in_genome)
          : regs(), inputs(), outputs(), 
          inst_ptr(0), flow_head(0), read_head(0), write_head(0),
          genome(in_genome), genome_working(in_genome){
        Initialize();
        ResetHardware();
      }
      // Create a default VirtualCPU (no genome sequence, default instruction set)
      VirtualCPU() : 
        VirtualCPU(genome_t(inst_lib_t::DefaultInstLib())) {
          Initialize();
         ResetHardware(); 
        }
      // Copy constructor
      VirtualCPU(const VirtualCPU &) = default;
      // Move constructor
      VirtualCPU(VirtualCPU &&) = default;
      // Destructor
      virtual ~VirtualCPU() { ; }
      

      //// GETTERS
      size_t GetSize() const { return genome.GetSize(); }
      size_t GetNumRegs() const { return num_regs; }
      size_t GetNumNops() const { return num_nops; }
      const std::unordered_map<int,data_t> & GetOutputs() const { return outputs; }
      Ptr<const inst_lib_t> GetInstLib() const { return genome.GetInstLib(); }


      //// SETTERS
      void SetInputs(const emp::vector<data_t> & vals) { 
        inputs = emp::ToUMap<int,data_t>(vals); 
      }
      

      //// GENOME / INSTRUCTION MANIPULATION 
      bool Load(std::istream & input) {
        File file(input);
        file.RemoveComments("--");  // Remove all comments beginning with -- (including --> and ----)
        file.RemoveComments("//");  // Remove all comments beginning with -- (including --> and ----)
        file.RemoveComments("#");  // Remove all comments beginning with -- (including --> and ----)
        file.CompressWhitespace();  // Trim down remaining whitespace.
        file.Apply( [this](std::string & info){ PushInst(info); } );
        return true;
      }
      bool Load(const std::string & filename) { 
        std::ifstream is(filename); 
        return Load(is); 
      }
      void PushInst(size_t idx){
        const size_t id = GetInstLib()->GetID(idx);
        genome.emplace_back(idx, id);
        genome_working.emplace_back(idx, id);
      }
      /// Add a new instruction to the end of the genome, by NAME and args.
      void PushInst(const std::string & name) {
        std::cout << "Pushing " << name << std::endl;
        PushInst(GetInstLib()->GetIndex(name));
      }
      /// Add a specified new instruction to the end of the genome.
      void PushInst(const inst_t & inst) { 
        genome.emplace_back(inst); 
        genome_working.emplace_back(inst); 
      }
      /// Add multiple copies of a specified instruction to the end of the genome.
      void PushInst(const inst_t & inst, size_t count) {
        genome.reserve(genome.size() + count);
        for (size_t i = 0; i < count; i++) genome.emplace_back(inst);
        genome_working.reserve(genome.size() + count);
        for (size_t i = 0; i < count; i++) genome_working.emplace_back(inst);
      }
      /// Add one or more default instructions to the end of the genome.
      void PushDefaultInst(size_t count=1) { 
        PushInst( inst_t(0, GetInstLib()->GetID(0)), count ); 
      }
      inst_t GetRandomInst(Random & rand) {
        size_t idx = rand.GetUInt(GetInstLib()->GetSize());
        size_t id = GetInstLib()->GetID(idx);
        return inst_t(idx, id);
      }
      void SetInst(size_t pos, const inst_t & inst) { 
        genome[pos] = inst; 
        genome_working[pos] = inst;
      }
      void RandomizeInst(size_t pos, Random & rand) { SetInst(pos, GetRandomInst(rand) ); }
      void PushRandom(Random & random, const size_t count=1) {
        for (size_t i = 0; i < count; i++) {
          PushInst(GetRandomInst(random));
        }
      }


      //// HEAD MANIPULATION
      void ResetIP(){
        inst_ptr = 0;
      }
      void AdvanceIP(size_t steps=1){
        inst_ptr += steps;
        inst_ptr %= genome_working.size();
      }
      void AdvanceRH(size_t steps=1){
        read_head += steps;
        read_head %= genome_working.size();
      }
      void AdvanceWH(size_t steps=1){
        write_head += steps;
        write_head %= genome_working.size();
      }
      void AdvanceFH(size_t steps=1){
        flow_head += steps;
        flow_head %= genome_working.size();
      }
      void SetIP(size_t pos){
        inst_ptr = pos;
        inst_ptr %= genome_working.size();
      }
      void SetRH(size_t pos){
        read_head = pos;
        read_head %= genome_working.size();
      }
      void SetWH(size_t pos){
        write_head = pos;
        write_head %= genome_working.size();
      }
      void SetFH(size_t pos){
        flow_head = pos;
        flow_head %= genome_working.size();
      }
      void SetModdedHead(size_t head_idx, size_t pos){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) SetIP(pos);
        else if(modded_idx == 1) SetRH(pos);
        else if(modded_idx == 2) SetWH(pos);
        else if(modded_idx == 3) SetFH(pos);
      }
      void AdvanceModdedHead(size_t head_idx, size_t pos){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) AdvanceIP(pos);
        else if(modded_idx == 1) AdvanceRH(pos);
        else if(modded_idx == 2) AdvanceWH(pos);
        else if(modded_idx == 3) AdvanceFH(pos);
      }
      size_t GetModdedHead(size_t head_idx){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) return inst_ptr;
        else if(modded_idx == 1) return read_head;
        else if(modded_idx == 2) return write_head;
        else if(modded_idx == 3) return flow_head;
      }


      //// HARDWARE MANIPULATION
      void Initialize(){
        CountNops();
        ExpandRegisters();
      }
      /// Reset just the CPU hardware, but keep the genome and traits.
      virtual void ResetHardware() {
        // Initialize registers to their posision.  So Reg0 = 0 and Reg11 = 11.
        for (size_t i = 0; i < num_regs; i++) {
          regs[i] = (data_t) i;
        }
        inputs.clear();
        outputs.clear();
        for(size_t i = 0; i < NUM_STACKS; ++i){
          stacks[i].resize(0);
        }
        inst_ptr = 0;           // Move heads back to beginning
        flow_head = 0;
        read_head = 0;
        write_head = 0;
        copied_inst_id_vec.clear();
        genome_working = genome;
       }
      /// Reset the entire CPU to a starting state, without a genome.
      void Reset() {
        genome.resize(0,0);    // Clear out genome
        genome_working.resize(0,0);    // Clear out genome
        ResetHardware();     // Reset the full hardware
      }
      void CurateNops(){ 
        bool label_inst_present = GetInstLib()->IsInst("Label");
        size_t label_inst_id = label_inst_present ? GetInstLib()->GetID("Label") : 0;

        if(!are_nops_counted) CountNops();
        label_idx_vec.clear();
        // Start by filling the nop vector of the last instruction
        for(size_t inst_idx = 0; inst_idx < genome_working.GetSize() - 1; ++inst_idx){
          if(emp::Has(nop_id_to_idx_map, genome_working[inst_idx].id))
            genome_working[genome_working.size() - 1].nop_vec.push_back(
                genome_working[inst_idx].id);
          else
            break;
        }
        // If the last index is a label, record it! 
        if(label_inst_present && 
            (genome_working[genome_working.size() - 1].id == label_inst_id))
          label_idx_vec.push_back(genome_working.size() - 1);
        // Now iterate backward over the genome, filling in each instruction's nop vector
        // Example, our genome looks like xyzabc where only a, b, and c are nops
        // If we are on index 2 (z), we see it is followed by a nop. 
        // Thus, we copy the next instruction into the nop vector [a]
        // Then we copy THAT instruction's nop vector, too: [a,b,c]
        // By going in reverse order, all following instructions already have a nop vec
        size_t inst_idx = 0;
        for(auto it = genome_working.rbegin() + 1; it != genome_working.rend(); ++it){
          if(emp::Has(nop_id_to_idx_map, (it - 1)->id)){
            it->nop_vec.resize( (it - 1)->nop_vec.size() + 1 );
            it->nop_vec[0] = (it - 1)->id;
            std::copy( 
                (it - 1)->nop_vec.begin(), 
                (it - 1)->nop_vec.end(), 
                it->nop_vec.begin() + 1);
           if(label_inst_present && (it->id == label_inst_id)) // Record pos if inst is label
              label_idx_vec.push_back(inst_idx);
           ++inst_idx;
          }
        }
        needs_nops_curated = false;
      }
      void CountNops(){
        std::cout << "\n\nCounting nops!" << std::endl;
        num_nops = 0;
        nop_id_to_idx_map.clear();
        nop_idx_to_id_map.clear();
        are_nops_counted = true;
        if(GetInstLib()->IsInst("NopA")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopA");
          nop_id_to_idx_map[id] = 0;
          nop_idx_to_id_map[0] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopB")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopB");
          nop_id_to_idx_map[id] = 1;
          nop_idx_to_id_map[1] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopC")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopC");
          nop_id_to_idx_map[id] = 2;
          nop_idx_to_id_map[2] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopD")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopD");
          nop_id_to_idx_map[id] = 3;
          nop_idx_to_id_map[3] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopE")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopE");
          nop_id_to_idx_map[id] = 4;
          nop_idx_to_id_map[4] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopF")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopF");
          nop_id_to_idx_map[id] = 5;
          nop_idx_to_id_map[5] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopG")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopG");
          nop_id_to_idx_map[id] = 6;
          nop_idx_to_id_map[6] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopH")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopH");
          nop_id_to_idx_map[id] = 7;
          nop_idx_to_id_map[7] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopI")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopI");
          nop_id_to_idx_map[id] = 8;
          nop_idx_to_id_map[8] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopJ")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopJ");
          nop_id_to_idx_map[id] = 9;
          nop_idx_to_id_map[9] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopK")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopK");
          nop_id_to_idx_map[id] = 10;
          nop_idx_to_id_map[10] = id;
        }
        else return;
        if(GetInstLib()->IsInst("NopL")){
          num_nops++;
          size_t id = GetInstLib()->GetID("NopL");
          nop_id_to_idx_map[id] = 11;
          nop_idx_to_id_map[11] = id;
        }
        else return;
      }
      void ExpandRegisters(){
        if(!are_nops_counted) CountNops();
        std::cout << "Expanding registers! " << "Nops: " << num_nops << std::endl;
        are_regs_expanded = true;
        num_regs = num_nops;
        regs.resize(num_regs);
      }    



      //// NOP SEQUENCE METHODS
      size_t GetComplementIdx(size_t idx){
        if(idx >= num_nops - 1) return 0;
        else return idx + 1;
      }
      nop_vec_t GetComplementLabel(const nop_vec_t& nop_vec){
        nop_vec_t res_vec;
        for(size_t nop : nop_vec){
          res_vec.push_back(GetComplementIdx(nop));
        }
        return res_vec;
      }
        bool CompareSequences(const nop_vec_t& search_vec, const nop_vec_t& compare_vec){
          if(search_vec.size() > compare_vec.size()) return false;
          if(search_vec.size() == 0 || compare_vec.size() == 0) return false;
          for(size_t idx = 0; idx < search_vec.size(); ++idx){
            if(search_vec[idx] != compare_vec[idx]) return false;
          }
          return true;
        }
        int FindLabel(const nop_vec_t& label, size_t start_idx){
          if(label.size() == 0) return -1;
          for(size_t offset = 1; offset < genome_working.size(); ++offset){
            size_t idx = start_idx + offset < genome_working.size() ?
              start_idx + offset : start_idx + offset - genome_working.size();
            if(CompareSequences(label, genome_working[idx].nop_vec)) return offset;
          }
          return -1;
        }
      int FindComplementLabel(const nop_vec_t& label, size_t start_idx){
        nop_vec_t comp_label = GetComplementLabel(label);
        return FindLabel(comp_label, start_idx);
      }
      bool CheckIfLastCopied(const nop_vec_t& label){
        if(label.size() > copied_inst_id_vec.size()) return false;
        if(label.size() == 0) return false;
        int idx = label.size() - 1;;
        for(auto copied_it = copied_inst_id_vec.rbegin(); copied_it != copied_inst_id_vec.rend(); copied_it++){
          if(*copied_it != label[idx])
            return false;
          idx--;
          if(idx < 0) break;

        }
        return true;
      }
      bool CheckIfLastCopiedComplement(const nop_vec_t& label){
        nop_vec_t comp_label = GetComplementLabel(label);
        return CheckIfLastCopied(comp_label);
      }
      size_t FindMarkedLabel_Reverse(bool start_local){
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_label_vec_idx =  label_idx_vec.size() - 1;
        if(start_local){
          bool start_found = false;
          for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
            if(label_idx_vec[label_idx_vec.size() - offset - 1] <= inst_ptr){
              start_found = true;
              break;
            }
          }
          if(!start_found) start_label_vec_idx = label_idx_vec.size() - 1;
          for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
            const size_t idx = 
              label_idx_vec[
                (start_label_vec_idx - offset + label_idx_vec.size()) % label_idx_vec.size()
              ];
            if(CompareSequences(search_vec, genome_working[idx].nop_vec)) return idx;
          }
        }
        return inst_ptr;
      }
      size_t FindMarkedLabel(bool start_local, bool reverse = false){
        if(reverse) FindMarkedLabel_Reverse(start_local);
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_label_vec_idx = 0;
        if(start_local){
          bool start_found = false;
          for(; start_label_vec_idx < label_idx_vec.size(); ++start_label_vec_idx){
            if(label_idx_vec[start_label_vec_idx] >= inst_ptr){
              start_found = true;
              break;
            }
          }
          if(!start_found) start_label_vec_idx = 0;
          for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
            const size_t idx = label_idx_vec[(start_label_vec_idx + offset) % label_idx_vec.size()];
            if(CompareSequences(search_vec, genome_working[idx].nop_vec)) return idx;
          }
        }
        return inst_ptr;
      }
      size_t FindSequence_Reverse(bool start_local){
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_idx = genome_working.size() - 1;
        if(start_local && inst_ptr != 0) start_idx = inst_ptr - 1;
        for(size_t offset = 0; offset < genome_working.size(); ++offset){
          const size_t idx = (start_idx - offset + genome_working.size()) % genome_working.size();
          if(CompareSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }
      size_t FindSequence(bool start_local, bool reverse = false){
        if(reverse) FindSequence_Reverse(start_local);
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_idx = 0;
        if(start_local) start_idx = inst_ptr + 1;
        for(size_t offset = 0; offset < genome_working.size(); ++offset){
          const size_t idx = (start_idx + offset) % genome_working.size(); 
          if(CompareSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }


      //// STACK MANIPULATION
      void StackPush(size_t reg_idx){
        stacks[active_stack_idx].push_back(regs[reg_idx]);
      }
      void StackPop(size_t reg_idx){
        if(stacks[active_stack_idx].size()){
          regs[reg_idx] = *stacks[active_stack_idx].rbegin();
          stacks[active_stack_idx].pop_back();
        }
      }
      void StackSwap(){
        active_stack_idx++;
        if(active_stack_idx >= NUM_STACKS) active_stack_idx = 0;
      }


      //// PROCESSING 
      // Process the NEXT instruction pointed to be the instruction pointer
      void SingleProcess(bool verbose = true) {
        emp_assert(genome_working.GetSize() > 0);  // A genome must exist to be processed.
        if(!are_regs_expanded) ExpandRegisters();
        if(needs_nops_curated) CurateNops();
        if(verbose){
          GetInstLib()->GetName(genome_working[inst_ptr].idx);
          PrintDetails();
        }
        GetInstLib()->ProcessInst(ToPtr(this), genome_working[inst_ptr]);
        AdvanceIP();
      }
      // Process the next SERIES of instructions, directed by the instruction pointer.
      void Process(size_t num_inst, bool verbose) { 
        for (size_t i = 0; i < num_inst; i++) SingleProcess(verbose); 
      }
    

      //// STATE TO STRING FUNCTIONS
      std::string GetString(){
        std::stringstream sstr;
        sstr << "[" << genome_working.size() << "]";
        for(size_t idx = 0; idx < genome_working.size(); idx++){
          unsigned char c = 'a' + genome_working[idx].id;
          if(genome_working[idx].id > 25) c = 'A' + genome_working[idx].id - 26;
          sstr << c;
        }
        return sstr.str();
      }
      std::string GetOriginalString(){
        std::stringstream sstr;
        sstr << "[" << genome.size() << "]";
        for(size_t idx = 0; idx < genome.size(); idx++){
          unsigned char c = 'a' + genome[idx].id;
          if(genome[idx].id > 25) c = 'A' + genome[idx].id - 26;
          sstr << c;
        }
        return sstr.str();
      }
      void PrintDetails(){
        std::cout << "IP: " << inst_ptr;
        std::cout << " RH: " << read_head;
        std::cout << " WH: " << write_head;
        std::cout << " FH: " << flow_head;
        std::cout << "(nops: " << num_nops << "; regs: " << num_regs << ")" << std::endl;
        for(size_t reg_idx = 0; reg_idx < regs.size(); ++reg_idx){
          std::cout << "[" << reg_idx << "] " << regs[reg_idx] << std::endl;
        }
        std::cout << std::endl;

      }
  
  }; // End VirtualCPU class
} // End namespace



#endif
