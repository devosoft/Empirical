/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  Population.h
 *  @brief Container for a collection of organisms of the same type.
 *
 *  A World may have many different types of organisms in it; all organisms of the same type
 *  are organized into a population so that they can all be manipulated inconcert.
 */

#ifndef MABE_POPULATION_H
#define MABE_POPULATION_H

#include "../../base/Ptr.h"
#include "../../base/vector.h"

#include "../base/ModuleBase.h"

namespace mabe {

  class PopulationBase : public ModuleBase {
  public:
    static constexpr mabe::ModuleType GetModuleType() { return ModuleType::POPULATION; }
  };

  template <typename ORG_T>
  class Population : public PopulationBase {
  public:
    using this_t = Population<ORG_T>;            ///< Resolved type of this templated class.
    using org_t = ORG_T;                         ///< Type of organisms in this population.
    using value_type = org_t;                    ///< Identical to org_t; vector compatibility.
    using vec_t = emp::vector<emp::Ptr<org_t>>;  ///< Type for storing all organisms.
    //using iterator_t = Pop_iterator<this_t>;     ///< Type for this world's iterators.

  };

}

#endif

