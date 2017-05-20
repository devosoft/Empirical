//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This file defines iterators for use with a PopulationManager in an emp::evo::World
//
//
//  Developer notes:
//  * It may be worth building a Const interator type to avoid unintended modifications.

#ifndef EMP_EVO_POP_ITER_H
#define EMP_EVO_POP_ITER_H

namespace emp {
namespace evo {

  template <typename POP_MANAGER>
  class PopulationIterator {
  private:
    using org_ptr_t = typename POP_MANAGER::value_type;

    POP_MANAGER * pop;
    int pos;

    // The MakeValid() function moves an iterator to t next non-null position (or the end)
    void MakeValid() {
      while ((pos < (int) pop->size()) && (((*pop)[(size_t)pos]) == nullptr)) ++pos;
    }

  public:
    PopulationIterator(POP_MANAGER * pm, int ind=0) : pop(pm), pos(ind) { MakeValid(); }
    PopulationIterator(const PopulationIterator & _in) : pop(_in.pop), pos(_in.pos) { MakeValid(); }

    PopulationIterator<POP_MANAGER> & operator=(const PopulationIterator & _in) {
      pop = _in.pop;
      pos = _in.pos;
      MakeValid();
      return *this;
    }

    PopulationIterator<POP_MANAGER> & operator++() {
      ++pos;
      MakeValid();
      return *this;
    }

    PopulationIterator<POP_MANAGER> & operator--() {
      --pos;
      while (pos < pop->size() && (((*pop)[(size_t)pos]) == nullptr)) { --pos; }
      return *this;
    }

    bool operator==(const PopulationIterator<POP_MANAGER>& rhs) const { return pos == rhs.pos; }
    bool operator!=(const PopulationIterator<POP_MANAGER>& rhs) const { return pos != rhs.pos; }
    bool operator< (const PopulationIterator<POP_MANAGER>& rhs) const { return pos <  rhs.pos; }
    bool operator<=(const PopulationIterator<POP_MANAGER>& rhs) const { return pos <= rhs.pos; }
    bool operator> (const PopulationIterator<POP_MANAGER>& rhs) const { return pos >  rhs.pos; }
    bool operator>=(const PopulationIterator<POP_MANAGER>& rhs) const { return pos >= rhs.pos; }

    auto operator*() -> decltype((*pop)[(size_t)pos]) { MakeValid(); return (*pop)[(size_t)pos]; }
    const org_ptr_t & operator*() const {
      MakeValid();
      return (org_ptr_t)(*pop)[(size_t)pos];
    }

    operator bool() const { MakeValid(); return pos < pop->size(); }

    PopulationIterator<POP_MANAGER> begin() {
      return PopulationIterator<POP_MANAGER>(pop, 0);
    }
    const PopulationIterator<POP_MANAGER> begin() const {
      return PopulationIterator<POP_MANAGER>(pop, 0);
    }

    PopulationIterator<POP_MANAGER> end() {
      return PopulationIterator<POP_MANAGER>(pop, pop->size());
    }
    const PopulationIterator<POP_MANAGER> end() const {
      return PopulationIterator<POP_MANAGER>(pop, pop->size());
    }

  };
}
}
#endif
