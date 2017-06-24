//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//  (Originally called PopulationIterator.h)
//
//  This file defines iterators for use with emp::World objects.
//
//
//  Developer notes:
//  * It may be worth building a Const interator type to avoid unintended modifications.

#ifndef EMP_EVO_WORLD_ITER_H
#define EMP_EVO_WORLD_ITER_H

#include "../base/Ptr.h"

namespace emp {

  template <typename ORG>
  class World_iterator {
  private:
    using org_t = ORG;
    using world_t = World<org_t>;
    using this_t = World_iterator<org_t>;

    Ptr<world_t> world_ptr;
    size_t pos;

    // WorldSize() is a shortcut to get the size of the pointed-to world object.
    size_t WorldSize() { emp_assert(world_ptr); return world_ptr->size(); }

    // The MakeValid() function moves an iterator to t next non-null position (or the end)
    void MakeValid() {
      while ((pos < WorldSize()) && (((*world_ptr)[pos]) == nullptr)) ++pos;
    }

  public:
    World_iterator(world_t * pm, int ind=0) : world_ptr(pm), pos(ind) { MakeValid(); }
    World_iterator(const World_iterator & _in) : world_ptr(_in.world_ptr), pos(_in.pos) { MakeValid(); }

    this_t & operator=(const World_iterator & _in) {
      world_ptr = _in.world_ptr;
      pos = _in.pos;
      MakeValid();
      return *this;
    }

    this_t & operator++() {
      ++pos;
      MakeValid();
      return *this;
    }

    this_t & operator--() {
      --pos;
      while (pos < WorldSize() && (((*world_ptr)[(size_t)pos]) == nullptr)) { --pos; }
      return *this;
    }

    bool operator==(const this_t& rhs) const { return pos == rhs.pos; }
    bool operator!=(const this_t& rhs) const { return pos != rhs.pos; }
    bool operator< (const this_t& rhs) const { return pos <  rhs.pos; }
    bool operator<=(const this_t& rhs) const { return pos <= rhs.pos; }
    bool operator> (const this_t& rhs) const { return pos >  rhs.pos; }
    bool operator>=(const this_t& rhs) const { return pos >= rhs.pos; }

    auto operator*() -> decltype((*world_ptr)[(size_t)pos]) { MakeValid(); return (*world_ptr)[(size_t)pos]; }
    const org_ptr_t & operator*() const {
      MakeValid();
      return (org_ptr_t)(*world_ptr)[(size_t)pos];
    }

    operator bool() const { MakeValid(); return pos < WorldSize(); }

    this_t begin() {
      return this_t(world_ptr, 0);
    }
    const this_t begin() const {
      return this_t(world_ptr, 0);
    }

    this_t end() {
      return this_t(world_ptr, WorldSize());
    }
    const this_t end() const {
      return this_t(world_ptr, WorldSize());
    }

  };

}

#endif
