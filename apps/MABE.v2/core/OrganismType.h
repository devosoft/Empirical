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
    using data_fun_t = std::function<typename genomes_data_t::template to_function_t<double>>;

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
    OrganismType(const std::string & in_name) : OrganismTypeBase(in_name) { 
      // Loop through all genome types.
      size_t genome_count = 0;
      emp::TupleIterate(genome_types, [this, &genome_count](GenomeBase & genome_type){
        genome_type.SetName( emp::to_string("genome", genome_count) );
        genome_count++;
        config.AddNameSpace(genome_type.GetConfig(), genome_type.GetName());
      });

      // Loop through all brain types.
      size_t brain_count = 0;
      emp::TupleIterate(brain_types, [this, &brain_count](BrainBase & brain_type){
        brain_type.SetName( emp::to_string("brain", brain_count) );
        brain_count++;
        config.AddNameSpace(brain_type.GetConfig(), brain_type.GetName());
      });

      std::cout << "OrganismType `" << GetName() << "' has "
                << genome_count << " genomes and "
                << brain_count << " brains."
                << std::endl;
    }    

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

    /// Add a new event function for this organism type; wrap the function and store it.
    bool AddEventFunction(FunctionInfo & info) override {
      std::function<double(OrganismBase &)> out_fun;
      // If the input can be handled by a brain, use it!
      // @CAO WRITE THIS!!

      // If the input wasn't handled by a brain, try out the genome state.
      if (info.fun_ptr->ConvertOK<data_fun_t>()) {
        auto genome_fun = *( info.fun_ptr->template Convert<data_fun_t>() );
        event_fun_t event_fun = [genome_fun](OrganismBase & org){
          ApplyTuple(genome_fun, ((Organism&) org).genomes);
        };
        event_funs.push_back(event_fun);
        return true;
      }

      // If we don't know how to deal with this info (i.e., we made it this far), return false.
      return false;
    }

    /// Add a new action function for this organism type; wrap the function and store it.
    bool AddActionFunction(FunctionInfo & info) override {
      std::function<double(OrganismBase &)> out_fun;
      // If the input can be handled by a brain, use it!
      // If the input wasn't handled by a brain, try out the genome state.
      // If we don't know how to deal with this info (i.e., we made it this far), return false.
      return false;
    }
  };

}

#endif

