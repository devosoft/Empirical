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
 * 
 *  On various type of operations, OrganismType will forward signals to brains and genomes.
 *  Specifically, it forwards the following functions:
 * 
 *    void Randomize(emp::Random & random, org_t & org)
 *    void Print(std::ostream & os, org_t & org) const
 *    void OnBeforeRepro(org_t & parent_org)
 *    void OnOffspringReady(org_t & parent_org, org_t & offspring_org)
 *    void OnInjectReady(org_t & org)
 *    void OnBeforePlacement(org_t & org)
 *    void OnPlacement(org_t & org)
 *    void OnOrgDeath(org_t & org)

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
    using this_t = OrganismType<Ts...>;
    using modules_t = emp::TypePack<Ts...>;
    using org_ptr_t = emp::Ptr<OrganismBase>;

    using genome_types_t = typename modules_t::template filter<is_genome_type>;
    using genome_types_tup_t = typename genome_types_t::template apply<std::tuple>;
    template <typename T> using to_genome_t = typename T::genome_t;
    using genomes_t = typename genome_types_t::template wrap<to_genome_t>;
    using genome_tup_t = typename genomes_t::template apply<std::tuple>;
    using genome_fun_t = std::function<typename genomes_t::template to_function_t<double>>;

    using brain_types_t = typename modules_t::template filter<is_brain_type>;
    using brain_types_tup_t  = typename brain_types_t::template apply<std::tuple>;
    template <typename T> using to_brain_t = typename T::brain_t;
    using brains_t = typename brain_types_t::template wrap<to_brain_t>;
    using brain_tup_t  = typename brains_t::template apply<std::tuple>;

    genome_types_tup_t genome_types;
    brain_types_tup_t brain_types;

    /// The configuration object for organisms is a set of namespaces for its components.
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
      static_assert( sizeof...(Ts) > 0, "Must have at least one brain -or- organism.");

      // Loop through all genome types.
      size_t genome_count = 0;
      emp::TupleIterate(genome_types, [this, &genome_count](GenomeTypeBase & genome_type){
        genome_type.SetName( emp::to_string("genome", genome_count) );
        genome_count++;
        config.AddNameSpace(genome_type.GetConfig(), genome_type.GetName());
      });

      // Loop through all brain types.
      size_t brain_count = 0;
      emp::TupleIterate(brain_types, [this, &brain_count](BrainTypeBase & brain_type){
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
      genome_tup_t genomes;
      brain_tup_t brains;
    public:
      Organism(emp::Ptr<this_t> type_ptr) : OrganismBase(type_ptr), genomes(), brains() { ; }
      Organism(emp::Ptr<this_t> type_ptr, emp::Random & random)
       : OrganismBase(type_ptr), genomes(), brains() { type_ptr->Randomize(random, *this); }
      Organism(const Organism & in_org) = default;
      Organism(Organism && in_org) = default;

      template<int ID> auto & GetGenome() { return std::get<ID>(genomes); }
      template<int ID> auto & GetBrain() { return std::get<ID>(brains); }
      const genome_tup_t & GetGenomes() const { return genomes; }
      const brain_tup_t & GetBrains() const { return brains; }
      genome_tup_t & GetGenomes() { return genomes; }
      brain_tup_t & GetBrains() { return brains; }

      emp::Ptr<OrganismBase> Clone() { return emp::NewPtr<Organism>(*this); }
    };

    void Randomize(emp::Random & random, Organism & org) {
      emp::TupleIterate(genome_types, org.GetGenomes(), [&random](auto & gtype, auto & genome) {
        gtype.Randomize(random, genome);
      });
    }

    org_ptr_t BuildOrg(emp::Random & random) override {
      return emp::NewPtr<Organism>(this,random);
    }

    void Print(std::ostream & os, OrganismBase & org) override {
      emp::TupleIterate(genome_types, ((Organism &) org).GetGenomes(), [&os](auto & gtype, auto & genome) {
        gtype.Print(os, genome);
      });
    }

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

    constexpr size_t GetNumGenomes() const { return genome_types_t::GetSize(); }
    constexpr size_t GetNumBrains() const { return brain_types_t::GetSize(); }

    void Setup(EvolverBase & mabe) override {
      default_org_data = mabe.GetOrgDataBlob();
    }

    template <typename RETURN, typename... ARGS>
    bool AddActionFunction(std::function<RETURN(ARGS...)> fun,
                           size_t action_id, const std::string & name,
                           const std::string & type, const std::string & desc)
    {
      // For the moment, ignore unused arguments.
      (void) name; (void) type; (void) desc;

      std::cout << "Trying to add function '" << name << "'." << std::endl;

      // Make sure we have room for this action.
      if (action_funs.size() <= action_id) action_funs.resize(action_id+1);
      emp_assert(action_funs[action_id] == false, "Trying to replace an existing action function.");

      // If this action can be used by a brain, do so!
      // @CAO Write this.

      // Otherwise, see if this action can be called with the genome.
      if constexpr ( std::is_same<genome_fun_t, std::function<RETURN(ARGS...)>>() ) {
        // Build a function that find the genome state of an organisms, feeds it into the input
        // function, and return the result.
        auto action_fun = [fun](OrganismBase & org_base) {
          Organism & org = (Organism &) org_base;
          return emp::ApplyTuple( fun, org.GetGenomes() );
        };

        // Store a pointer to this function, converted to a GenericFunction
        action_funs[action_id] = emp::NewPtr< emp::Function<RETURN(OrganismBase &)>>(action_fun);

        std::cout << "SUCCESS!" << std::endl;
        return true;
      }

      // Otherwise we won't use it.
      std::cout << "FAILURE!" << std::endl;
      return false;
    }

    /// Add a new event function for this organism type; wrap the function and store it.
    // bool AddEventFunction(FunctionInfo & info) override {
    //   std::function<double(OrganismBase &)> out_fun;
    //   // If the input can be handled by a brain, use it!
    //   // @CAO WRITE THIS!!

    //   // If the input wasn't handled by a brain, try out the genome state.
    //   if (info.fun_ptr->ConvertOK<genome_fun_t>()) {
    //     auto genome_fun = *( info.fun_ptr->template Convert<genome_fun_t>() );
    //     event_fun_t event_fun = [genome_fun](OrganismBase & org){
    //       ApplyTuple(genome_fun, ((Organism&) org).genomes);
    //     };
    //     event_funs.push_back(event_fun);
    //     return true;
    //   }

    //   // If we don't know how to deal with this info (i.e., we made it this far), return false.
    //   return false;
    // }

    // /// Add a new action function for this organism type; wrap the function and store it.
    // bool AddActionFunction(FunctionInfo & info) override {
    //   std::function<double(OrganismBase &)> out_fun;
    //   // If the input can be handled by a brain, use it!
    //   // If the input wasn't handled by a brain, try out the genome state.
    //   // If we don't know how to deal with this info (i.e., we made it this far), return false.
    //   return false;
    // }
  };

}

#endif

