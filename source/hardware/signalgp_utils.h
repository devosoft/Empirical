/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  signalgp_utils.h
 *  @brief Helper functions for working with SignalGP virtual hardware/programs.
 *  
 *  @todo Generate random function/program/instruction/tag (as separate helper functions)
      - [x] Tag
      - [x] Tags
      - [x] Instruction
      - [x] Function
      - [x] Program
 *  @todo Generate random tags (w/uniqueness)
 *  @todo Mutator class
 *  @todo tests
 */

#ifndef EMP_SIGNALGP_UTILS_H
#define EMP_SIGNALGP_UTILS_H

#include <unordered_set>

#include "base/errors.h"
#include "hardware/EventDrivenGP.h"
#include "tools/BitSet.h"
#include "tools/math.h"

namespace emp {

  /// Generate one random SignalGP tag (BitSet<TAG_WIDTH>). Given a vector of other tags (unique_from), this 
  /// function will guarantee the tag generated is unique with respect to those tags. 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param unique_from - Other tags that the tag being generated should be unique with respect to. 
  template<size_t TAG_WIDTH>
  BitSet<TAG_WIDTH> GenRandSignalGPTag(emp::Random & rnd, const emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
    using tag_t = BitSet<TAG_WIDTH>;  
    emp_assert(unique_from.size() < emp::Pow2(TAG_WIDTH), "Tag width is not large enough to be able to guarantee requested number of unique tags"); 
    tag_t new_tag(rnd, 0.5); // Make a random tag.
    bool guarantee_unique = (bool)unique_from.size();
    while (guarantee_unique) {
      guarantee_unique = false;
      for (size_t i = 0; i < unique_from.size(); ++i) {
        if (unique_from[i] == new_tag) {
          guarantee_unique = true;
          new_tag.Randomize(rnd);
          break;
        }
      }
    }
    return new_tag;
  }


  /// Generate 'count' number of random SignalGP tags (BitSet<TAG_WIDTH>). 
  /// Given a vector of other tags (unique_from), this function will guarantee the tags generated are unique with respect to those tags. 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param count - How many tags should be generated? 
  /// @param guarantee_unique - Should generated tags be guaranteed to be unique from each other and from tags in 'unique_from'?
  /// @param unique_from - Other tags that the tag being generated should be unique with respect to. Only used if 'guarantee_unique' is true. 
  template<size_t TAG_WIDTH>
  emp::vector<BitSet<TAG_WIDTH>> GenRandSignalGPTags(emp::Random & rnd, size_t count, bool guarantee_unique=false, 
                                                     const emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
    using tag_t = BitSet<TAG_WIDTH>;  
    emp_assert(!guarantee_unique || (unique_from.size()+count <= emp::Pow2(TAG_WIDTH)), "Tag width is not large enough to be able to guarantee requested number of unique tags"); 

    std::unordered_set<uint32_t> uset; // Used to ensure all generated tags are unique.
    emp::vector<tag_t> new_tags;
    for (size_t i = 0; i < unique_from.size(); ++i) uset.emplace(unique_from[i].GetUInt(0));
    for (size_t i = 0; i < count; ++i) {
      new_tags.emplace_back(tag_t());
      new_tags[i].Randomize(rnd);
      if (guarantee_unique) {
        uint32_t tag_int = new_tags[i].GetUInt(0);
        while (true) {
          if (!emp::Has(uset, tag_int)) {
            uset.emplace(tag_int);
            break;
          } else {
            new_tags[i].Randomize(rnd);
            tag_int = new_tags[i].GetUInt(0);
          }
        }
      }
    }
    return new_tags;
  }

  /// Generate a random SignalGP instruction (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the instruction (instruction will be valid within instruction library)
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH> 
  typename EventDrivenGP_AW<TAG_WIDTH>::Instruction GenRandSignalGPInst(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, size_t min_arg_val=0, size_t max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    using inst_t = typename EventDrivenGP_AW<TAG_WIDTH>::Instruction;
    using tag_t = BitSet<TAG_WIDTH>;
    return inst_t(rnd.GetUInt(inst_lib.GetSize()),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  rnd.GetInt(min_arg_val, max_arg_val+1),
                  tag_t(rnd, 0.5));       
  }

  /// Generate a random SignalGP function (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the function (instruction will be valid within instruction library)
  /// @param min_inst_cnt - Minimum number of instructions in generated function.
  /// @param max_inst_cnt - Maximum number of instructions in generated function.
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH>
  typename EventDrivenGP_AW<TAG_WIDTH>::Function GenRandSignalGPFunction(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, 
                                                                            size_t min_inst_cnt=1, size_t max_inst_cnt=32, 
                                                                            size_t min_arg_val=0, size_t max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    using fun_t = typename EventDrivenGP_AW<TAG_WIDTH>::Function;
    size_t inst_cnt = rnd.GetUInt(min_inst_cnt, max_inst_cnt+1);
    fun_t new_fun(emp::GenRandSignalGPTag<TAG_WIDTH>(rnd));
    for (size_t i = 0; i < inst_cnt; ++i) new_fun.PushInst(emp::GenRandSignalGPInst(rnd, inst_lib, min_arg_val, max_arg_val));
    return new_fun;
  }

  /// Generate a random SignalGP program (templated off of tag width). 
  /// @param rnd - Random number generator to use when generating a random tag. 
  /// @param inst_lib - Instruction library used to generate the program. 
  /// @param min_func_cnt - Mininum number of functions in generated program.
  /// @param max_func_cnt - Maximum number of functions in generated program.
  /// @param min_inst_cnt - Minimum number of instructions per function.
  /// @param max_inst_cnt - Maximum number of instructions per function.
  /// @param min_arg_val - Mininum value for an instruction argument.
  /// @param max_arg_val - Maximum value for an instruction argument.
  template<size_t TAG_WIDTH>
  typename EventDrivenGP_AW<TAG_WIDTH>::Program GenRandSignalGPProgram(emp::Random & rnd, const emp::InstLib<EventDrivenGP_AW<TAG_WIDTH>> & inst_lib, 
                                                                       size_t min_func_cnt=1, size_t max_func_cnt=16,
                                                                       size_t min_inst_cnt=1, size_t max_inst_cnt=32, 
                                                                       size_t min_arg_val=0, size_t max_arg_val=15) {
    emp_assert(inst_lib.GetSize() > 0, "Instruction library must have at least one instruction definition before being used to generate a random instruction.");
    using program_t = typename EventDrivenGP_AW<TAG_WIDTH>::Program;
    program_t program(&inst_lib);
    size_t fun_cnt = rnd.GetUInt(min_func_cnt, max_func_cnt+1);
    for (size_t f = 0; f < fun_cnt; ++f) 
      program.PushFunction(emp::GenRandSignalGPFunction(rnd, inst_lib, min_inst_cnt, max_inst_cnt, min_arg_val, max_arg_val));
    return program;  
  }

}

#endif