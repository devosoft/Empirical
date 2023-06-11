/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file Trait.hpp
 *  @brief The TraitDef class maintains a category of measuments about another class.
 *
 *  Each trait is associated with a name, a description, and a type.  Instance of that
 *  trait are of type TraitValue.  A TraitManager contains information about a group of
 * related traits, and a TraitSet is a set of TraitValues.
 */

#ifndef EMP_IN_PROGRESS_TRAIT_HPP_INCLUDE
#define EMP_IN_PROGRESS_TRAIT_HPP_INCLUDE

#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "../base/assert.hpp"
#include "../meta/meta.hpp"

namespace emp {

  template <typename TRAIT_TYPE> struct TraitKey {
    int index;
    TraitKey(int _index) : index(_index) { ; }
  };

  template <typename TRAIT_TYPE>
  class TraitDef {
  private:
    const std::string name;
    const std::string desc;
    const TRAIT_TYPE default_val;
    const int index;

  public:
    TraitDef(const std::string & _name, const std::string & _desc, const TRAIT_TYPE & _default_val,
             int _index)
      : name(_name), desc(_desc), default_val(_default_val), index(_index)
    { ; }
    ~TraitDef() { ; }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    const TRAIT_TYPE & GetDefault() const { return default_val; }
    int GetIndex() const { return index; }
    TraitKey<TRAIT_TYPE> GetKey() const { return index; }
  };

  template <typename... TRAIT_TYPES> class TraitManager;

  template <typename... TRAIT_TYPES>
  class TraitSet {
    friend class TraitManager<TRAIT_TYPES...>;
  private:
    std::tuple< std::vector<TRAIT_TYPES>... > type_sets;

    // Add in a new trait (with value) to the appropriate type set.
    template <typename IN_TYPE>
    void PushTrait(const IN_TYPE & in_trait) {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      static_assert(type_id >= 0, "Unhandled type provided to TraitSet::PushTrait()");
      std::get<type_id>(type_sets).push_back(in_trait);
    }

    // Get set (vector of entries) associated with the given type.
    template <typename IN_TYPE>
    std::vector<IN_TYPE> & GetTypeSet() {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      static_assert(type_id >= 0, "Unknown type provided to TraitSet::GetTypeSet()");
      return std::get< type_id >(type_sets);
    }

  public:
    TraitSet(const TraitManager<TRAIT_TYPES...> & tm); // Defined after TraitManager.

    // Access a specific trait value by passing in its definition.
    template <typename IN_TYPE>
    const IN_TYPE & Get(const TraitKey<IN_TYPE> & in_key) const {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      static_assert(type_id >= 0, "Unknown type provided to TraitSet::Get() const");
      return std::get<type_id>(type_sets)[in_key.GetIndex()];
    }

    template <typename IN_TYPE>
    IN_TYPE & Get(const TraitKey<IN_TYPE> & in_key) {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      static_assert(type_id >= 0, "Unknown type provided to TraitSet::Get()");
      return std::get<type_id>(type_sets)[in_key.GetIndex()];
    }
  };

  template <typename... TRAIT_TYPES>
  class TraitManager {
    friend class TraitSet<TRAIT_TYPES...>;
  private:
    // A group of trait definitions must be created for each type handled.
    std::tuple< std::vector< TraitDef<TRAIT_TYPES> >... > trait_groups;
    int num_traits;
    TraitSet<TRAIT_TYPES...> default_trait_set;

    static const int num_types = sizeof...(TRAIT_TYPES);

    // Helper Functions:

    // Return a constant indicating the position of a given type in the tuple.
    template <typename BASE_TYPE>
    constexpr static int GetTraitID() {
      return emp::get_type_index<BASE_TYPE, TRAIT_TYPES...>();
    }

    // Return the vector of traits for the given type (const version).
    template <typename IN_TYPE>
    const std::vector< TraitDef<IN_TYPE> > & GetTraitGroup() const {
      static_assert(GetTraitID<IN_TYPE>() >= 0,
                    "Unknown type provided to TraitManager::GetTraitGroup() const");
      return std::get< GetTraitID<IN_TYPE>() >(trait_groups);
    }

    // Return the vector of traits for the given type.
    template <typename IN_TYPE>
    std::vector< TraitDef<IN_TYPE> > & GetTraitGroup() {
      static_assert(GetTraitID<IN_TYPE>() >= 0,
                    "Unknown type provided to TraitManager::GetTraitGroup()");
      return std::get< GetTraitID<IN_TYPE>() >(trait_groups);
    }

    // Base Case...
    template <typename CUR_TYPE>
    void SetDefaultsByType(TraitSet<TRAIT_TYPES...> & trait_set) const {
      // Get the relevant vectors for the current type.
      const std::vector< TraitDef<CUR_TYPE> > & cur_group = GetTraitGroup<CUR_TYPE>();
      std::vector<CUR_TYPE> & type_set = trait_set.template GetTypeSet<CUR_TYPE>();

      // Set all of the values in type_set
      type_set.resize(0);
      for (const TraitDef<CUR_TYPE> & cur_def : cur_group) {
        type_set.push_back(cur_def.GetDefault());
      }
    }

    template <typename FIRST_TYPE, typename SECOND_TYPE, typename... OTHER_TYPES>
    void SetDefaultsByType(TraitSet<TRAIT_TYPES...> & trait_set) const {
      SetDefaultsByType<FIRST_TYPE>(trait_set);

      // And recurse through the other types.
      SetDefaultsByType<SECOND_TYPE, OTHER_TYPES...>(trait_set);
    }

  public:
    TraitManager() : num_traits(0), default_trait_set(*this) { ; }
    ~TraitManager() { ; }

    static int GetNumTypes() { return num_types; }

    int GetNumTraits() const { return num_traits; }
    template <typename IN_TYPE> int GetNumTraitsOfType() const {
      return (int) GetTraitGroup<IN_TYPE>().size();
    }

    // Lookup a trait by its type and index.
    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & GetTraitDef(int index) {
      std::vector< TraitDef<IN_TYPE> > & cur_group = GetTraitGroup<IN_TYPE>();
      emp_assert(index >= 0 && index < (int) cur_group.size());
      return cur_group[index];
    }

    // Lookup a trait by its type and index.
    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & GetTraitDef(TraitKey<IN_TYPE> key) {
      std::vector< TraitDef<IN_TYPE> > & cur_group = GetTraitGroup<IN_TYPE>();
      return cur_group[key.GetIndex()];
    }

    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & AddTrait(const std::string & _name, const std::string & _desc,
                              const IN_TYPE & _default_val) {
      std::vector< TraitDef<IN_TYPE> > & cur_group = GetTraitGroup<IN_TYPE>();
      const int trait_index = (int) cur_group.size();
      cur_group.push_back( TraitDef<IN_TYPE>(_name, _desc, _default_val, trait_index) );
      num_traits++;
      return cur_group[trait_index];
    }

    void SetDefaults(TraitSet<TRAIT_TYPES...> & trait_set) const {
      SetDefaultsByType<TRAIT_TYPES...>(trait_set);
    }
  };


  template <typename... TRAIT_TYPES>
  TraitSet<TRAIT_TYPES...>::TraitSet(const TraitManager<TRAIT_TYPES...> & tm) {
    tm.SetDefaults(*this);
  }

}

#endif // #ifndef EMP_IN_PROGRESS_TRAIT_HPP_INCLUDE
