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

  template <typename ENV_T> class Evolver;


  /// A vector that can be indexed with an EvolverID
  class WorldVector : public emp::array<emp::vector< emp::Ptr<mabe::OrganismBase> >, 2> {
  public:
    using ptr_t = emp::Ptr<mabe::OrganismBase>;
    using base_t = emp::array<emp::vector< ptr_t >, 2>;

    /// Test if an id is currently valid.
    bool IsValid(EvolverID pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      return id < base_t::operator[](pop_id).size();
    }

    /// Make sure ID is in vector range; if not expand relevant vector.
    void MakeValid(EvolverID pos) {
      emp_assert(pos.IsValid());
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      emp_assert(pop_id < 2);
      if (id >= base_t::operator[](pop_id).size()) {
        base_t::operator[](pop_id).resize(id+1);
      }
    }

    ptr_t & operator()(EvolverID pos) {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
    const ptr_t & operator()(EvolverID pos) const {
      const size_t pop_id = pos.GetPopID();
      const size_t id = pos.GetIndex();
      return base_t::operator[](pop_id)[id];
    }
  };

}

#endif
