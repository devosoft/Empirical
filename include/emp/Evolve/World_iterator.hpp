/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2017-2018
 *  @note Originally called PopulationIterator.h
 *
 *  @file  World_iterator.hpp
 *  @brief This file defines iterators for use with emp::World objects.
 *
 *  @todo Add a const interator.
 *  @todo Currently we do MakeValid after every change AND before many accesses.  Pick one?
 *  @todo Fix operator-- which can go off of the beginnig of the world.
 */

#ifndef EMP_EVO_WORLD_ITER_H
#define EMP_EVO_WORLD_ITER_H

#include "../base/Ptr.hpp"

namespace emp {

  template <typename WORLD>
  class World_iterator {
  private:
    using world_t = WORLD;
    using org_t = typename world_t::org_t;
    using this_t = World_iterator<world_t>;

    Ptr<world_t> world_ptr;
    size_t pos;

    // WorldSize() is a shortcut to get the size of the pointed-to world object.
    size_t WorldSize() { emp_assert(world_ptr); return world_ptr->size(); }

    // OrgPtr() is a shortcut to retrieve an organism from the pointed-to world object.
    Ptr<org_t> OrgPtr() { emp_assert(world_ptr); return world_ptr->pop[pos]; }

    // The MakeValid() function moves an iterator to t next non-null position (or the end)
    void MakeValid() {
      while (pos < WorldSize() && OrgPtr() == nullptr) ++pos;
    }

  public:
    /// Create an iterator in the specified world pointing to the first occupied cell after the
    /// provided start position.
    World_iterator(world_t * _w, size_t _p=0) : world_ptr(_w), pos(_p) { MakeValid(); }

    /// Create an iterator pointing to the same position as another iterator.
    World_iterator(const World_iterator & _in) : world_ptr(_in.world_ptr), pos(_in.pos) { MakeValid(); }

    /// Assign this iterator to the position of another iterator.
    this_t & operator=(const World_iterator & _in) {
      world_ptr = _in.world_ptr;
      pos = _in.pos;
      MakeValid();
      return *this;
    }

    /// Advance iterator to the next non-empty cell in the world.
    this_t & operator++() {
      ++pos;
      MakeValid();
      return *this;
    }

    /// Backup iterator to the previos non-empty cell in the world.
    this_t & operator--() {
      --pos;
      while (pos < WorldSize() && OrgPtr() == nullptr) --pos;
      return *this;
    }

    /// Compare two iterators to determine if they point to identical positions in the world.
    bool operator==(const this_t& rhs) const { return pos == rhs.pos; }

    /// Compare two iterators to determine if they point to different positions in the world.
    bool operator!=(const this_t& rhs) const { return pos != rhs.pos; }

    /// Determine if this iterator points to a position in the world BEFORE another iterator.
    bool operator< (const this_t& rhs) const { return pos <  rhs.pos; }

    /// Determine if this iterator points to a position in the world BEFORE or the SAME AS another iterator.
    bool operator<=(const this_t& rhs) const { return pos <= rhs.pos; }

    /// Determine if this iterator points to a position in the world AFTER another iterator.
    bool operator> (const this_t& rhs) const { return pos >  rhs.pos; }

    /// Determine if this iterator points to a position in the world AFTER or the SAME AS another iterator.
    bool operator>=(const this_t& rhs) const { return pos >= rhs.pos; }

    /// Return a reference to the organism pointed to by this iterator.
    org_t & operator*() { MakeValid(); return *(OrgPtr()); }

    /// Return a const reference to the organism pointed to by this iterator.
    const org_t & operator*() const { MakeValid(); return *(OrgPtr()); }

    /// Is this iterator pointing to a valid cell in the world?
    operator bool() const { MakeValid(); return pos < WorldSize(); }

    /// Return an iterator pointing to the first occupied cell in the world.
    this_t begin() { return this_t(world_ptr, 0); }

    /// Return a const iterator pointing to the first occupied cell in the world.
    const this_t begin() const { return this_t(world_ptr, 0); }

    /// Return an iterator pointing to just past the end of the world.
    this_t end() { return this_t(world_ptr, WorldSize()); }

    /// Return a const iterator pointing to just past the end of the world.
    const this_t end() const { return this_t(world_ptr, WorldSize()); }
  };

}

#endif
