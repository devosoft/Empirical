/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  World_structure.h
 *  @brief Classes and functions for maintaining various world structure methods.
 */

#ifndef MABE_WORLD_STRUCTURE_H
#define MABE_WORLD_STRUCTURE_H

#include <set>

#include "base/array.h"
#include "base/assert.h"
#include "base/vector.h"

#include "data/Trait.h"

#include "tools/math.h"
#include "tools/Random.h"
#include "tools/vector_utils.h"

namespace mabe {

  class World;

  /// A class to track positions in World.
  /// For the moment, the only informaiton beyond index is active (vs. next) population when
  /// using synchronous generations.
  //
  //  Developer NOTE: For efficiency, internal class members are uint32_t, but to prevent compiler
  //                  warnings, size_t values are accepted; asserts ensure safe conversions.
  class WorldPosition {
  private:
    uint32_t index;   ///<  Position of this organism in the population.
    uint32_t pop_id;  ///<  ID of the population we are in; 0 is always the active population.

  public:
    static constexpr size_t invalid_id = (uint32_t) -1;

    WorldPosition() : index(invalid_id), pop_id(invalid_id) { ; }
    WorldPosition(size_t _id, size_t _pop_id=0) : index((uint32_t) _id), pop_id((uint32_t) _pop_id) {
      emp_assert(_id <= invalid_id);
      emp_assert(_pop_id <= invalid_id);
    }
    WorldPosition(const WorldPosition &) = default;

    uint32_t GetIndex() const { return index; }
    uint32_t GetPopID() const { return pop_id; }

    bool IsActive() const { return pop_id == 0; }
    bool IsValid() const { return index != invalid_id; }

    WorldPosition & SetActive(bool _active=true) { pop_id = _active ? 0 : 1; return *this; }
    WorldPosition & SetPopID(size_t _id) { emp_assert(_id <= invalid_id); pop_id = (uint32_t) _id; return *this; }
    WorldPosition & SetIndex(size_t _id) { emp_assert(_id <= invalid_id); index = (uint32_t) _id; return *this; }
    WorldPosition & MarkInvalid() { index = invalid_id; pop_id = invalid_id; return *this; }
  };

  /// A vector that can be indexed with a WorldPosition
  class WorldVector : public emp::array<emp::vector< emp::Ptr<mabe::OrganismBase> >, 2> {
  public:
    using ptr_t = emp::Ptr<mabe::OrganismBase>;
    using base_t = emp::array<emp::vector< ptr_t >, 2>;

    /// Test if a position is currently valid.
    bool IsValid(WorldPosition pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      return id < base_t::operator[](pop_id).size();
    }

    /// Make sure position is valid; if not expand relevant vector.
    void MakeValid(WorldPosition pos) {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      if (id >= base_t::operator[](pop_id).size()) {
        base_t::operator[](pop_id).resize(id+1);
      }
    }

    ptr_t & operator()(WorldPosition pos) {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
    const ptr_t & operator()(WorldPosition pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
  };

}

#endif
