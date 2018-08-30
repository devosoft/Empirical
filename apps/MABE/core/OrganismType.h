/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  OrganismType.h
 *  @brief Template to construct organisms from brains and genomes.
 *
 *  Organisms can be made directly -OR- be built using zero or more brains (controllers) and
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
    using genomes_tup_t = typename genomes_t::template apply<std::tuple>;
    template <typename T> using to_data = typename T::data_t;
    using genomes_data_t = typename genomes_t::template wrap<to_data>;
    using data_tup_t = typename genomes_data_t::template apply<std::tuple>;

    using brains_t = typename modules_t::template filter<is_brain>;
    using brains_tup_t  = typename brains_t::template apply<std::tuple>;
    template <typename T> using to_compute = typename T::compute_t;
    using brains_compute_t = typename brains_t::template wrap<to_compute>;
    using compute_tup_t  = typename brains_compute_t::template apply<std::tuple>;

    genomes_tup_t genome_types;
    brains_tup_t brain_types;

    /// The configuration object for organisms is a set of namespaces for its components.
    // @CAO: Setup these namespaces!
    emp::Config config;

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

    class Organism : public OrganismBase {
    private:
      data_tup_t genomes;
      compute_tup_t brains;
    public:
      template<int ID> auto & GetGenome() { return std::get<ID>(genomes); }
      template<int ID> auto & GetBrain() { return std::get<ID>(brains); }
    };

    /// Print out the name of this class, including template parameters (for debugging)
    std::string GetClassName() const override {
      return emp::to_string("OrganismType<", GetModuleClassNames<Ts...>(), ">");
    }

    /// Required accessor for configuration objects.
    emp::Config & GetConfig() override { return config; }

    /// Access a specific genome 
    template<int ID>
    auto & GetGenomeType() { return std::get<ID>(genome_types); }

    template<int ID>
    auto & GetBrainType() { return std::get<ID>(brain_types); }

  };

}

#endif

