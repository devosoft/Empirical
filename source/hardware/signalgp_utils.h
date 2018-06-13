/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  signalgp_utils.h
 *  @brief Helper functions for working with SignalGP virtual hardware/programs.
 *  
 *  @todo Generate random function/program/instruction/tag (as separate helper functions)
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
  BitSet<TAG_WIDTH> GenRandTag(emp::Random & rnd, emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
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
  emp::vector<BitSet<TAG_WIDTH>> GenRandTags(emp::Random & rnd, size_t count, bool guarantee_unique=false, 
                                             emp::vector<BitSet<TAG_WIDTH>> & unique_from=emp::vector<BitSet<TAG_WIDTH>>()) {
    using tag_t = BitSet<TAG_WIDTH>;  
    emp_assert(unique_from.size()+count < emp::Pow2(TAG_WIDTH), "Tag width is not large enough to be able to guarantee requested number of unique tags"); 

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
            tag_int = tags[i].GetUInt(0);
          }
        }
      }
    }
    return new_tags;
  }
  
}

#endif