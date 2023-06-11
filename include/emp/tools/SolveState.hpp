/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file SolveState.hpp
 *  @brief Used as part of a branching solver to keep track of the current state.
 *  @note Status: BETA
 */

#ifndef EMP_TOOLS_SOLVESTATE_HPP_INCLUDE
#define EMP_TOOLS_SOLVESTATE_HPP_INCLUDE


#include "../base/assert.hpp"

#include "../bits/BitVector.hpp"

namespace emp {

  /// Often in a branch-and-bound algorithm, we need to identify the sub-set of items that
  /// maximizes (or minimizes) an optimization metric.  SolveState keeps track of the current
  /// state for which items have been locked in as "included" in the current branks, which have
  /// been "excluded", and which are still "unknown" (still to be decided upon.)
  /// All tracking is performed with BitVectors for high efficiency.

  class SolveState {
  private:
    BitVector in_items;   ///< Items included for sure
    BitVector unk_items;  ///< Items yet to be decided on

  public:
    SolveState(size_t state_size=0) : in_items(state_size), unk_items(state_size) {
      unk_items.SetAll();
    }
    SolveState(const SolveState & in) : in_items(in.in_items), unk_items(in.unk_items) { ; }
    ~SolveState() { ; }

    /// Set this SolveState to be identical to another.
    SolveState & operator=(const SolveState & in) {
      in_items = in.in_items;
      unk_items = in.unk_items;
      return *this;
    }

    /// How many items are being considered in the current SolveState?
    size_t GetSize() const { return in_items.GetSize(); }

    /// Test if a particular item is going to be included for sure in the current solve state.
    /// (If it has been excluded -OR- is yet to be decided upon, false will be returned)
    bool IsIn(size_t id) const { return in_items[id]; }

    /// Test if a particular item is yet to be decided upon in the current solve state.
    /// (If it has been excluded -OR- is included for sure, false will be returned)
    bool IsUnk(size_t id) const { return unk_items[id]; }

    /// Test if a particular item is going to be excluded for sure in the current solve state.
    /// (If it has been included -OR- is yet to be decided upon, false will be returned)
    bool IsOut(size_t id) const { return !(IsIn(id) | IsUnk(id)); }

    /// Test if all items have been decided upon (none are still in the "unknown" state)
    bool IsFinal() const { return unk_items.None(); }

    /// How many items have been included for sure?
    size_t CountIn() const { return in_items.CountOnes(); }

    /// How many items have yet to be decided upon (are "unknown")
    size_t CountUnk() const { return unk_items.CountOnes(); }

    /// How many items have been excluded for sure.
    size_t CountOut() const { return in_items.GetSize() - CountIn() - CountUnk(); }

    /// Get the BitVector associated with which items have been included for sure.
    const BitVector & GetInVector() const { return in_items; }

    /// Get the BitVector associated with which items have yet to be decided upon.
    const BitVector & GetUnkVector() const { return unk_items; }

    /// Get the BitVector associated with which iterm have been excluded for sure.
    BitVector GetOutVector() const { return ~in_items & ~unk_items; }

    /// Get the ID of the next unknown item.
    int GetNextUnk(size_t prev_unk) const {
      return unk_items.FindOne(prev_unk+1);
    }

    /// Mark a specific item as to be included.
    void Include(size_t id) {
      emp_assert(id >= 0 && id < in_items.size());
      unk_items.Set(id, false);
      in_items.Set(id, true);
    }

    /// Mark a specific item as to be excluded.
    void Exclude(size_t id) {
      emp_assert(id >= 0 && id < in_items.size());
      unk_items.Set(id, false);
    }

    /// Change our mind about a potentially included node (Be careful since many algorithms don't
    /// requite this type of changes to be made.)
    void ForceExclude(size_t id) {
      unk_items.Set(id, false);
      in_items.Set(id, false);
    }

    /// Include ALL of the items specified in the provided BitVector.
    void IncludeSet(const BitVector & inc_set) {
      emp_assert(inc_set.GetSize() == in_items.GetSize());
      in_items |= inc_set;
      unk_items &= ~inc_set;
    }

    /// Exclude ALL of the items specified in the provided BitVector.
    void ExcludeSet(const BitVector & inc_set) {
      emp_assert(inc_set.GetSize() == in_items.GetSize());
      unk_items &= ~inc_set;
    }
  };

}

#endif // #ifndef EMP_TOOLS_SOLVESTATE_HPP_INCLUDE
