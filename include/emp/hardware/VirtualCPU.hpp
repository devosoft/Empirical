/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file VirtualCPU.hpp
 *  @brief A simple virtual CPU styled after the original and extended Avidian architectures.
 *
 *  @TODO
 *    - Expanded heads?
 *    - expanded_nop_args useful?
 *    - Consider changing default return value for search functions
 *    - Consider switching to (or adding an optional mode) where nops are only curated
 *          as-needed instead of all at once
 *
 */

#ifndef EMP_HARDWARE_VIRTUALCPU_HPP_INCLUDE
#define EMP_HARDWARE_VIRTUALCPU_HPP_INCLUDE

#include <fstream>
#include <iostream>
#include <map>

#include "../base/array.hpp"
#include "../base/Ptr.hpp"
#include "../base/unordered_map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/map_utils.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../io/File.hpp"
#include "../math/Random.hpp"
#include "../tools/string_utils.hpp"

#include "Genome.hpp"
#include "VirtualCPU_InstLib.hpp"

namespace emp{
  /// \brief A simple virtual CPU styled after those seen in Avida
  ///
  /// This class represents a single virtual CPU following a genome of assembly-level
  /// instructions.
  /// By defaullt, eaach CPU features four heads, two stacks, multiple registers, and
  /// a circular genome.
  /// Both the original and extended architectures are supported.
  template <typename DERIVED>
  class VirtualCPU{
    public:
      static constexpr size_t NUM_STACKS = 2; ///< Number of stacks in this CPU (currently 2)
      static constexpr size_t MAX_NOPS = 23; ///< Maximum number of nop instructions supported
      struct Instruction;

      using derived_t = DERIVED;
      using data_t = uint32_t;
      using inst_t = Instruction;
      using inst_lib_t = VirtualCPU_InstLib<derived_t, data_t, 0>;
      using genome_t = Genome<Instruction, inst_lib_t>;
      using nop_vec_t = emp::vector<size_t>;
      using stack_t = emp::vector<data_t>;

      /// \brief Representation of a single instruction in the CPU's genome
      ///
      /// Only contains the necessary information for which instruction is being represented
      /// as well as any data it needs in the genome.
      /// Does NOT contain the actual logic of the instruction, nor the name.
      /// These are handled by the instruction library itself.
      struct Instruction : public inst_lib_t::InstructionBase {
        size_t idx;                  /// Index of the instruction in the instruction library
        size_t id;                   /// Identifier for the instruction that gives the user
                                     /// flexibility over the instruction (e.g., what symbol
                                     /// it should use in a string representation)
        emp::vector<size_t> nop_vec; /// Representation of the contiguous sequence of NOP
                                     /// instructions following this instruction in the genome

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

        size_t GetIndex() const override { return idx; }
      };


    protected:
      size_t num_regs = 0;  ///< Number of registers found in this CPU
      size_t num_nops = 0;  ///< Number of NOP instructions found in this CPU's library

    public:
      //////// FLAGS
      bool are_nops_counted = false;   ///< Flag detailing if the number of NOP instructions
                                       ///< in the CPU's library have been counted
      bool are_regs_expanded = false;  ///< Flag signaling if the number of registers have
                                       ///< been expanded to accomodate the number of NOP
                                       ///< instructions in the library
      bool nops_need_curated = true;   ///< Flag signaling that NOP instructions need curated
      bool expanded_nop_args = false;  ///< Flag signaling that CPU is used the expanded

      //////// CPU COMPONENTS
      emp::vector<data_t> regs;                ///< Vector of registers
      std::unordered_map<int, data_t> inputs;  ///< Map of all available inputs
                                               ///< (position -> value)
      std::unordered_map<int, data_t> outputs; ///< Map of all outputs (position -> value)
      emp::array<stack_t, NUM_STACKS> stacks;  ///< Array of stacks for this CPU
      size_t inst_ptr;                         ///< Instruction pointer, signifies next
                                               ///< instruction to be executed
      size_t flow_head;                        ///< Flow head, used for moving heads and
                                               ///< values
      size_t read_head;                        ///< Read head, signals what instruction to
                                               ///< copy next
      size_t write_head;                       ///< Write head, signals where to copy next
                                               ///< instruction
      //////// HELPER CONSTRUCTS
      emp::unordered_map<size_t, size_t> nop_id_map;/**< NOP inst id -> Nop index
                                                         (e.g., NopA -> 0, NopB -> 1,
                                                           NopE -> 5) */
      emp::vector<size_t> label_idx_vec; ///< Vector of LABEL instructions indices in genome
      //////// GENOME
      genome_t genome;         ///< Preserved copy of genome from organism creation/birth
                               ///< that should not change in any way
      genome_t genome_working; ///< Working copy of genome that can mutate, resize, and change
      //////// BOOKKEEPING
      size_t active_stack_idx = 0;             ///< Index of CPU's active stack
      emp::vector<size_t> copied_inst_id_vec;  ///< Vector of instructions that have been
                                               ///< copied
      size_t num_insts_executed = 0;


      //////// CONSTRUCTORS / DESTRUCTOR
      /// Create a new VirtualCPU with the same genome (and thus instruction library)
      VirtualCPU(const genome_t & in_genome)
          : regs(), inputs(), outputs(),
          inst_ptr(0), flow_head(0), read_head(0), write_head(0),
          genome(in_genome), genome_working(in_genome){
        Initialize();
        ResetHardware();
      }
      /// Create a default VirtualCPU (no genome sequence, default instruction set)
      VirtualCPU() :
          VirtualCPU(genome_t(inst_lib_t::DefaultInstLib())) {
        Initialize();
        ResetHardware();
      }
      /// Create a perfect copy of passed VirtualCPU
      VirtualCPU(const VirtualCPU &) = default;
      /// Default move constructor
      VirtualCPU(VirtualCPU &&) = default;
      /// Default destructor
      virtual ~VirtualCPU() { ; }


      //////// GETTERS
      /// Return size of original genome
      size_t GetGenomeSize() const { return genome.GetSize(); }
      /// Return size of working genome
      size_t GetWorkingGenomeSize() const { return genome_working.GetSize(); }
      /// Return the number of registers in the CPU
      size_t GetNumRegs() const { return num_regs; }
      /// Return the number of NOP instructions found in the CPU's instruction library
      size_t GetNumNops() const { return num_nops; }
      /// Return the outputs of the CPU
      const std::unordered_map<int,data_t> & GetOutputs() const { return outputs; }
      /// Return a pointer to the CPU's instruction library
      Ptr<const inst_lib_t> GetInstLib() const { return genome.GetInstLib(); }


      //////// SETTERS
      /// Copies passed vector into input map
      void SetInputs(const emp::vector<data_t> & vals) {
        inputs = emp::ToUMap<int,data_t>(vals);
      }


      //////// GENOME & INSTRUCTION MANIPULATION
      /// Load instructions from input stream
      bool Load(std::istream & input) {
        ClearGenome();
        File file(input);
        file.RemoveComments("//"); // Remove all C++ style comments
        file.RemoveComments("#");  // Remove all bash/Python/R style comments
        file.CompressWhitespace();  // Trim down remaining whitespace.
        file.Apply( [this](std::string & info){ PushInst(info); } );
        nops_need_curated = true;
        return true;
      }
      /// Load instructions from file
      bool Load(const std::string & filename) {
        std::ifstream is(filename);
        return Load(is);
      }
      /// Add a new instruction to the end of the genome, by index in the instruction library
      void PushInst(size_t idx){
        const size_t id = GetInstLib()->GetID(idx);
        genome.emplace_back(idx, id);
        genome_working.emplace_back(idx, id);
        nops_need_curated = true;
      }
      /// Add a new instruction to the end of the genome, by name
      void PushInst(const std::string & name) {
        PushInst(GetInstLib()->GetIndex(name));
        nops_need_curated = true;
      }
      /// Add a specified new instruction to the end of the genome
      void PushInst(const inst_t & inst) {
        genome.emplace_back(inst);
        genome_working.emplace_back(inst);
        nops_need_curated = true;
      }
      /// Add multiple copies of a specified instruction to the end of the genome
      void PushInst(const inst_t & inst, size_t count) {
        genome.reserve(genome.size() + count);
        for (size_t i = 0; i < count; i++) genome.emplace_back(inst);
        genome_working.reserve(genome.size() + count);
        for (size_t i = 0; i < count; i++) genome_working.emplace_back(inst);
        nops_need_curated = true;
      }
      /// Return the first instruction in the instruction library
      inst_t GetDefaultInst(){
        return inst_t(0, GetInstLib()->GetID(0));
      }
      /// Add one or more default instructions to the end of the genome
      void PushDefaultInst(size_t count=1) {
        PushInst( inst_t(0, GetInstLib()->GetID(0)), count );
        nops_need_curated = true;
      }
      /// Return a random instruction from the instruction library
      inst_t GetRandomInst(Random & rand) {
        size_t idx = rand.GetUInt(GetInstLib()->GetSize());
        size_t id = GetInstLib()->GetID(idx);
        return inst_t(idx, id);
      }
      /// Overwrite the instruction at the given genome index with passed instruction
      void SetInst(size_t pos, const inst_t & inst) {
        genome[pos] = inst;
        genome_working[pos] = inst;
        nops_need_curated = true;
      }
      /// Overwrite the instruction at the given genome index with a random instruction
      void RandomizeInst(size_t pos, Random & rand) {
        SetInst(pos, GetRandomInst(rand) );
        nops_need_curated = true;
      }
      /// Add a random instruction from the instruction library to the end of the genome
      void PushRandomInst(Random & random, const size_t count=1) {
        for (size_t i = 0; i < count; i++) {
          PushInst(GetRandomInst(random));
        }
        nops_need_curated = true;
      }
      /// Insert the given instruction at the specified genome position
      void InsertInst(const inst_t& inst, const size_t idx){
        genome.emplace(genome.begin() + idx, inst);
        genome_working.emplace(genome_working.begin() + idx, inst);
        nops_need_curated = true;
      }
      /// Inserts a random instruction at the given genome position
      void InsertRandomInst(const size_t idx, emp::Random& random){
        InsertInst(GetRandomInst(random), idx);
      }
      /// Remove the instruction at the specified genome position
      void RemoveInst(const size_t idx){
        genome.erase(genome.begin() + idx);
        genome_working.erase(genome_working.begin() + idx);
        nops_need_curated = true;
      }



      //////// HEAD MANIPULATION
      /// Move the instruction pointer to the beginning of the genome
      void ResetIP(){
        inst_ptr = 0;
      }
      /// Move the read head to the beginning of the genome
      void ResetRH(){
        read_head = 0;
      }
      /// Move the write head to the beginning of the genome
      void ResetWH(){
        write_head = 0;
      }
      /// Move the flow head to the beginning of the genome
      void ResetFH(){
        flow_head = 0;
      }
      /// Advance the instruction pointer so many steps and wrap around the end of the genome
      void AdvanceIP(size_t steps=1){
        inst_ptr += steps;
        inst_ptr = (genome_working.size() > 0 ? inst_ptr % genome_working.size() : 0);
      }
      /// Advance the read head so many steps and wrap around the end of the genome
      void AdvanceRH(size_t steps=1){
        read_head += steps;
        read_head = (genome_working.size() > 0 ? read_head % genome_working.size() : 0);
      }
      /// Advance the write head so many steps and wrap around the end of the genome
      void AdvanceWH(size_t steps=1){
        write_head += steps;
        write_head = (genome_working.size() > 0 ? write_head % genome_working.size() : 0);
      }
      /// Advance the flow head so many steps and wrap around the end of the genome
      void AdvanceFH(size_t steps=1){
        flow_head += steps;
        flow_head = (genome_working.size() > 0 ? flow_head % genome_working.size() : 0);
      }
      /// Set the instruction pointer to the genome index, wrap around the end of the genome
      void SetIP(size_t pos){
        inst_ptr = pos;
        inst_ptr %= genome_working.size();
      }
      /// Set the read head to the genome index, wrap around the end of the genome
      void SetRH(size_t pos){
        read_head = pos;
        read_head %= genome_working.size();
      }
      /// Set the write head to the genome index, wrap around the end of the genome
      void SetWH(size_t pos){
        write_head = pos;
        write_head %= genome_working.size();
      }
      /// Set the flow head to the genome index, wrap around the end of the genome
      void SetFH(size_t pos){
        flow_head = pos;
        flow_head %= genome_working.size();
      }
      /// Set the specified head (which can wrap) to the beginning of the genom,
      void ResetModdedHead(size_t head_idx){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) SetIP(0);
        else if(modded_idx == 1) SetRH(0);
        else if(modded_idx == 2) SetWH(0);
        else if(modded_idx == 3) SetFH(0);
      }
      /// Set the specified head (which can wrap) to the given genome position,
      /// wrap around the end of the genome
      void SetModdedHead(size_t head_idx, size_t pos){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) SetIP(pos);
        else if(modded_idx == 1) SetRH(pos);
        else if(modded_idx == 2) SetWH(pos);
        else if(modded_idx == 3) SetFH(pos);
      }
      /// Advance the specified head (which can wrap) the given number of instructions,
      /// wrap around the end of the genome
      void AdvanceModdedHead(size_t head_idx, size_t steps=1){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) AdvanceIP(steps);
        else if(modded_idx == 1) AdvanceRH(steps);
        else if(modded_idx == 2) AdvanceWH(steps);
        else if(modded_idx == 3) AdvanceFH(steps);
      }
      /// Return the head POSITION of the specified head (can wrap)
      size_t GetModdedHead(size_t head_idx){
        size_t modded_idx = head_idx % 4;
        if(modded_idx == 0) return inst_ptr;
        else if(modded_idx == 1) return read_head;
        else if(modded_idx == 2) return write_head;
        else if(modded_idx == 3) return flow_head;
        return inst_ptr;
      }


      //////// HARDWARE MANIPULATION
      /// Initializes the CPU by counting the number of NOP instructions in the instruction
      /// library and expanding the number of registers to match
      void Initialize(){
        CountNops();
        ExpandRegisters();
        ResetHardware();
      }
      /// Reset all heads
      void ResetHeads(){
        ResetIP();
        ResetRH();
        ResetWH();
        ResetFH();
      }
      /// Reset all inputs and outputs
      void ResetIO(){
        inputs.clear();
        outputs.clear();
      }
      /// Reset all memory/data
      void ResetMemory(){
        // Initialize registers to their position.  So Reg0 = 0 and Reg11 = 11.
        for (size_t i = 0; i < num_regs; i++) {
          regs[i] = (data_t) i;
        }
        for(size_t i = 0; i < NUM_STACKS; ++i){
          stacks[i].resize(0);
        }
        active_stack_idx = 0;
      }
      /// Reset all bookkeeping variables
      void ResetBookkeeping(){
        copied_inst_id_vec.clear();
        num_insts_executed = 0;
      }
      /// Reset the working genome back to the original genome
      void ResetWorkingGenome(){
        genome_working = genome;
        label_idx_vec.clear();
        nops_need_curated = true;
      }
      /// Reset just the CPU hardware, but keep the original genome
      virtual void ResetHardware() {
        ResetHeads();
        ResetMemory();
        ResetIO();
        ResetBookkeeping();
      }
      /// Clear the main genome of the organism and reset all hardware
      void ClearGenome() {
        genome.resize(0,0);         // Clear out genome
        genome_working.resize(0,0); // Clear out working genome
        label_idx_vec.clear();      // No labels if genome is empty
        nops_need_curated = true;
        ResetHardware();            // Reset the full hardware
      }
      /// Compile NOP instructions in genome into useful nop vectors for each instruction,
      /// and records the position of all LABEL instructions
      void CurateNops(){
        if(genome_working.size() == 0){
          nops_need_curated = false;
          return;
        }
        bool label_inst_present = GetInstLib()->IsInst("Label");
        size_t label_inst_id = label_inst_present ? GetInstLib()->GetID("Label") : 0;

        if(!are_nops_counted) CountNops();
        label_idx_vec.clear();
        // Start by filling the nop vector of the last instruction
        for(size_t inst_idx = 0; inst_idx < genome_working.GetSize() - 1; ++inst_idx){
          if(emp::Has(nop_id_map, genome_working[inst_idx].id)){
            genome_working[genome_working.size() - 1].nop_vec.push_back(
                nop_id_map[genome_working[inst_idx].id]);
          }
          else break;
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
        for(auto it = genome_working.rbegin() + 1; it != genome_working.rend(); ++it){
          if(emp::Has(nop_id_map, (it - 1)->id)){
            it->nop_vec.resize( (it - 1)->nop_vec.size() + 1 );
            it->nop_vec[0] = nop_id_map[(it - 1)->id];
            std::copy(
                (it - 1)->nop_vec.begin(),
                (it - 1)->nop_vec.end(),
                it->nop_vec.begin() + 1);
          }
        }
        for(size_t inst_idx = 0; inst_idx < genome_working.size(); ++inst_idx){
           if(genome_working[inst_idx].id == label_inst_id) // Record pos if inst is label
              label_idx_vec.push_back(inst_idx);
        }
        nops_need_curated = false;
      }
      /// Determine the number of sequential NOP instructions in the instruction library
      ///
      /// Starts at NopA and continues from there. Any missing instructions force count to
      /// stop. Last possible NOP instruction is NopW, as NopX is a special case in Avida.
      void CountNops(){
        num_nops = 0;
        nop_id_map.clear();
        are_nops_counted = true;
        for(size_t idx = 0; idx < MAX_NOPS ; ++idx){ // Stop before X!
          std::string nop_name = (std::string)"Nop" + (char)('A' + idx);
          if(GetInstLib()->IsInst(nop_name)){
            num_nops++;
            size_t id = GetInstLib()->GetID(nop_name);
            nop_id_map[id] = idx;
          }
          else return;
        }
      }
      /// Expand the CPU's registers to match the number of NOP instructions in the
      /// instruction library
      void ExpandRegisters(){
        if(!are_nops_counted) CountNops();
        are_regs_expanded = true;
        num_regs = num_nops;
        regs.resize(num_regs);
      }

      //////// NOP SEQUENCE METHODS
      /// For a given NOP instruction (as an index), return its complement index
      size_t GetComplementNop(size_t idx){
        if(idx >= num_nops - 1) return 0;
        else return idx + 1;
      }
      /// For a vector of NOP instructions (as indices), return a vector of complement indices
      /// in the same order
      nop_vec_t GetComplementNopSequence(const nop_vec_t& nop_vec){
        nop_vec_t res_vec;
        for(size_t nop : nop_vec){
          res_vec.push_back(GetComplementNop(nop));
        }
        return res_vec;
      }
      /// Check if a vector of NOP instructions is the same as the START of another vector
      bool CompareNopSequences(const nop_vec_t& search_vec, const nop_vec_t& compare_vec){
        if(search_vec.size() > compare_vec.size()) return false;
        if(search_vec.size() == 0 || compare_vec.size() == 0) return false;
        for(size_t idx = 0; idx < search_vec.size(); ++idx){
          if(search_vec[idx] != compare_vec[idx]) return false;
        }
        return true;
      }
      /// Check if the given vector of NOP instructions (as indices) were the last
      /// instructions to be copied by the CPU
      bool CheckIfLastCopied(const nop_vec_t& label){
        if(label.size() > copied_inst_id_vec.size()) return false;
        if(label.size() == 0) return false;
        int idx = label.size() - 1;
        for(auto copied_it = copied_inst_id_vec.rbegin(); copied_it != copied_inst_id_vec.rend(); copied_it++){
          if(*copied_it != label[idx])
            return false;
          idx--;
          if(idx < 0) break;

        }
        return true;
      }
      /// Search up the genome (backward) for a sequence of NOP instructions following a LABEL
      /// instruction that match the NOP sequence following the current instruction
      ///
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      size_t FindLabel_Reverse(bool start_local){
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_label_vec_idx =  label_idx_vec.size() - 1;
        if(start_local){
          bool start_found = false;
          for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
            if(label_idx_vec[label_idx_vec.size() - offset - 1] < inst_ptr){
              start_label_vec_idx = label_idx_vec.size() - offset - 1;
              start_found = true;
              break;
            }
          }
          if(!start_found) start_label_vec_idx = label_idx_vec.size() - 1;
        }
        for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
          const size_t idx =
            label_idx_vec[
              (start_label_vec_idx - offset + label_idx_vec.size()) % label_idx_vec.size()
            ];
          if(CompareNopSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }
      /// Search the genome for a sequence of NOP instructions following a LABEL
      /// instruction that match the NOP sequence following the current instruction
      ///
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      /// @param reverse If true, traverse the genome backward. If false, traverse forward
      size_t FindLabel(bool start_local, bool reverse = false){
        if(reverse) return FindLabel_Reverse(start_local);
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        size_t start_label_vec_idx = 0;
        if(start_local){
          bool start_found = false;
          for(; start_label_vec_idx < label_idx_vec.size(); ++start_label_vec_idx){
            if(label_idx_vec[start_label_vec_idx] > inst_ptr){
              start_found = true;
              break;
            }
          }
          if(!start_found) start_label_vec_idx = 0;
        }
        for(size_t offset = 0; offset < label_idx_vec.size(); ++offset){
          const size_t idx = label_idx_vec[(start_label_vec_idx + offset) % label_idx_vec.size()];
          if(CompareNopSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }
      /// Search up the genome (backward) for a sequence of NOP instructions
      /// that match the given NOP sequence
      ///
      /// @param search_vec The sequence of NOP instructions to search for
      /// @param start_idx Position in the genom to start the search
      size_t FindNopSequence_Reverse(const nop_vec_t& search_vec, size_t start_idx){
        for(size_t offset = 1; offset < genome_working.size() + 1; ++offset){
          const size_t idx = (start_idx - offset + genome_working.size()) % genome_working.size();
          if(CompareNopSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }
      /// Search up the genome (backward) for a sequence of NOP instructions
      /// that match the given NOP sequence
      ///
      /// @param search_vec The sequence of NOP instructions to search for
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      size_t FindNopSequence_Reverse(const nop_vec_t& search_vec, bool start_local){
        size_t start_idx = 0;
        if(start_local && inst_ptr != 0) start_idx = inst_ptr;
        return FindNopSequence_Reverse(search_vec, start_idx);
      }
      /// Search up the genome (backward) for a sequence of NOP instructions
      /// that match the NOP sequence following the current instruction
      ///
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      size_t FindNopSequence_Reverse(bool start_local){
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        return FindNopSequence_Reverse(search_vec, start_local);
      }
      /// Search the genome for a sequence of NOP instructions that match the given
      /// NOP sequence
      ///
      /// @param search_vec The sequence of NOP instructions to search for
      /// @param start_idx Position in the genom to start the search
      size_t FindNopSequence(const nop_vec_t& search_vec, size_t start_idx,
          bool reverse = false){
        if(reverse) return FindNopSequence_Reverse(search_vec, start_idx);
        for(size_t offset = 1; offset < genome_working.size() + 1; ++offset){
          const size_t idx = (start_idx + offset) % genome_working.size();
          if(CompareNopSequences(search_vec, genome_working[idx].nop_vec)) return idx;
        }
        return inst_ptr;
      }
      /// Search the genome for a sequence of NOP instructions that match the given
      /// NOP sequence
      ///
      /// @param search_vec The sequence of NOP instructions to search for
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      /// @param reverse If true, traverse the genome backward. If false, traverse forward
      size_t FindNopSequence(const nop_vec_t& search_vec, bool start_local,
          bool reverse = false){
        size_t start_idx = genome_working.size() - 1;
        if(start_local) start_idx = inst_ptr;
        return FindNopSequence(search_vec, start_idx, reverse);
      }
      /// Search up the genome (backward) for a sequence of NOP instructions
      /// that match the NOP sequence following the current instruction
      ///
      /// @param start_local If true, search from instruction pointer. If false, search from
      /// start of the genome
      /// @param reverse If true, traverse the genome backward. If false, traverse forward
      size_t FindNopSequence(bool start_local, bool reverse = false){
        const nop_vec_t search_vec = genome_working[inst_ptr].nop_vec;
        return FindNopSequence(search_vec, start_local, reverse);
      }


      //////// STACK MANIPULATION
      /// Push the value in the specified register on top of the active stack
      void StackPush(size_t reg_idx){
        stacks[active_stack_idx].push_back(regs[reg_idx]);
      }
      /// Remove the value from the top of the active stack and store it in the
      /// specified register
      void StackPop(size_t reg_idx){
        if(stacks[active_stack_idx].size()){
          regs[reg_idx] = *stacks[active_stack_idx].rbegin();
          stacks[active_stack_idx].pop_back();
        }
      }
      /// Swap which stack is active
      void StackSwap(){
        active_stack_idx++;
        if(active_stack_idx >= NUM_STACKS) active_stack_idx = 0;
      }
      /// Fetch the nth value of the specified stack
      data_t GetStackVal(size_t stack_idx, size_t val_idx){
        emp_assert(stack_idx < NUM_STACKS);
        emp_assert(val_idx < stacks[stack_idx].size());
        size_t reverse_idx = stacks[stack_idx].size() - val_idx - 1;
        return stacks[stack_idx][reverse_idx];
      }


      //////// PROCESSING
      /// Process the next instruction pointed to be the instruction pointer
      void SingleProcess(bool verbose = true) {
        emp_assert(genome_working.GetSize() > 0);  // A genome must exist to be processed.
        if(!are_regs_expanded) ExpandRegisters();
        if(nops_need_curated) CurateNops();
        if(verbose){
          GetInstLib()->GetName(genome_working[inst_ptr].idx);
          PrintDetails();
        }
        GetInstLib()->ProcessInst(ToPtr(this), genome_working[inst_ptr]);
        AdvanceIP();
        num_insts_executed++;
      }
      /// Process the next SERIES of instructions, directed by the instruction pointer.
      void Process(size_t num_inst = 1, bool verbose = true) {
        for (size_t i = 0; i < num_inst; i++) SingleProcess(verbose);
      }


      //////// STATE -> STRING FUNCTIONS
      /// Return the working genome in string form.
      ///
      /// Each instruction is represented by a single character, dictated by the
      /// instruction's ID.
      std::string GetWorkingGenomeString(){
        std::stringstream sstr;
        sstr << "[" << genome_working.size() << "]";
        for(size_t idx = 0; idx < genome_working.size(); idx++){
          unsigned char c = 'a' + genome_working[idx].id;
          if(genome_working[idx].id > 25) c = 'A' + genome_working[idx].id - 26;
          sstr << c;
        }
        return sstr.str();
      }
      /// Return the original genome in string form.
      ///
      /// Each instruction is represented by a single character, dictated by the
      /// instruction's ID.
      std::string GetGenomeString(){
        std::stringstream sstr;
        sstr << "[" << genome.size() << "]";
        for(size_t idx = 0; idx < genome.size(); idx++){
          unsigned char c = 'a' + genome[idx].id;
          if(genome[idx].id > 25) c = 'A' + genome[idx].id - 26;
          sstr << c;
        }
        return sstr.str();
      }
      /// Output the state of the CPU's heads and registers to the specified output stream
      void PrintDetails(std::ostream& ostr = std::cout){
        ostr << "IP: " << inst_ptr;
        ostr << " RH: " << read_head;
        ostr << " WH: " << write_head;
        ostr << " FH: " << flow_head;
        ostr << "(nops: " << num_nops << "; regs: " << num_regs << ")" << std::endl;
        for(size_t reg_idx = 0; reg_idx < regs.size(); ++reg_idx){
          ostr << "[" << reg_idx << "] " << regs[reg_idx] << std::endl;
        }
      }

  }; // End VirtualCPU class
} // End namespace



#endif // #ifndef EMP_HARDWARE_VIRTUALCPU_HPP_INCLUDE
