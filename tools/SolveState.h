//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-17.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  SolveState is used as part of a branching solver to keep track of the current state.
//  Status: BETA

#ifndef EMP_SOLVE_STATE_H
#define EMP_SOLVE_STATE_H

#include "../base/assert.h"

#include "BitVector.h"

namespace emp {

  class SolveState {
  private:
    BitVector in_items;   // Objects included for sure
    BitVector unk_items;  // Objects yet to be decided on

  public:
    SolveState(size_t state_size=0) : in_items(state_size), unk_items(state_size) {
      unk_items.SetAll();
    }
    SolveState(const SolveState & in) : in_items(in.in_items), unk_items(in.unk_items) { ; }
    ~SolveState() { ; }

    SolveState & operator=(const SolveState & in) {
      in_items = in.in_items;
      unk_items = in.unk_items;
      return *this;
    }

    size_t GetSize() const { return in_items.GetSize(); }

    bool IsIn(size_t id) const { return in_items[id]; }
    bool IsUnk(size_t id) const { return unk_items[id]; }
    bool IsOut(size_t id) const { return !(IsIn(id) | IsUnk(id)); }

    bool IsFinal() const { return unk_items.None(); }

    size_t CountIn() const { return in_items.CountOnes(); }
    size_t CountUnk() const { return unk_items.CountOnes(); }
    size_t CountOut() const { return in_items.GetSize() - CountIn() - CountUnk(); }

    const BitVector & GetInVector() const { return in_items; }
    const BitVector & GetUnkVector() const { return unk_items; }
    BitVector GetOutVector() const { return ~in_items & ~unk_items; }

    int GetNextUnk(size_t prev_unk) const {
      return unk_items.FindBit(prev_unk+1);
    }

    void Include(size_t id) {
      emp_assert(id >= 0 && id < in_items.size());
      unk_items.Set(id, false);
      in_items.Set(id, true);
    }

    void Exclude(size_t id) {
      emp_assert(id >= 0 && id < in_items.size());
      unk_items.Set(id, false);
    }

    // Change our mind about a potentially included node
    void ForceExclude(size_t id) {
      unk_items.Set(id, false);
      in_items.Set(id, false);
    }

    void IncludeSet(const BitVector & inc_set) {
      emp_assert(inc_set.GetSize() == in_items.GetSize());
      in_items |= inc_set;
      unk_items &= ~inc_set;
    }

    void ExcludeSet(const BitVector & inc_set) {
      emp_assert(inc_set.GetSize() == in_items.GetSize());
      unk_items &= ~inc_set;
    }
  };

}

#endif
