/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismType.h
 *  @brief Template to construct organisms from controllers and genomes.
 *
 *  Organisms can be made directly -OR- be built using zero or more controllerts (brains) and
 *  zero or more genomes.  An OrgaanismType template will automatically handle that assembly.
 */

#ifndef MABE_ORGANISM_TYPE_H
#define MABE_ORGANISM_TYPE_H

#include "meta/TypePack.h"
#include "tools/string_utils.h"

#include "OrganismTypeBase.h"

namespace mabe {
  template <typename... Ts>
  class OrganismType : public OrganismTypeBase {
  private:
    using modules_t = emp::TypePack<Ts...>;
    using genomes_t = typename modules_t::template filter<is_genome>;
    using controllers_t = typename modules_t::template filter<is_controller>;

    using genomes_tup_t     = typename genomes_t::template apply<std::tuple>;
    using controllers_tup_t = typename controllers_t::template apply<std::tuple>;

    /// Collect the class names of internal modules.
    template <typename T>
    std::string GetModuleClassNames() const { T tmp_module; return tmp_module.GetClassName(); }
    template <typename T1, typename T2, typename... EXTRA>
    std::string GetModuleClassNames() const {
      return emp::to_string( GetModuleClassNames<T1>(), ",", GetModuleClassNames<T2, EXTRA...>() );
    }

    // @CAO: Some kind of switchboard should be established here for organism setup.


  public:
    OrganismType(const std::string & in_name) : OrganismTypeBase(in_name) { ; }    

    /// Print out the name of this class, including template parameters (for debugging)
    std::string GetClassName() const override {
      return emp::to_string("OrganismType<", GetModuleClassNames<Ts...>(), ">");
    }
  };

}

#endif

