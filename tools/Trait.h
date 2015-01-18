#ifndef EMP_TRAIT_H
#define EMP_TRAIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  The TraitDef class maintains a category of measuments about another class.
//
//  Each trait is associated with a name, a description, and a type.  Instance of that
//  trait are of type TraitValue.  A TraitManager contains information about a group of
//  related traits, and a TraitSet is a set of TraitValues.
//

#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "functions.h"

namespace emp {

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
  };

  template <typename... TRAIT_TYPES> class TraitManager;

  template <typename... TRAIT_TYPES>
  class TraitSet {
    friend class TraitManager<TRAIT_TYPES...>;
  private:
    std::tuple< std::vector<TRAIT_TYPES>... > type_sets;

    template <typename IN_TYPE>
    void PushTrait(const IN_TYPE & in_trait) {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      std::get<type_id>(type_sets).push_back(in_trait);
    }

    template <typename IN_TYPE>
    std::vector<IN_TYPE> & GetTypeSet() {
      constexpr int trait_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      return std::get< trait_id >(type_sets);
    }

  public:
    TraitSet(const TraitManager<TRAIT_TYPES...> & tm); // Defined after TraitManager.

    // Access a specific trait value by passing in its definition.
    template <typename IN_TYPE>
    IN_TYPE & Get(const TraitDef<IN_TYPE> & in_def) {
      constexpr int type_id = emp::get_type_index<IN_TYPE, TRAIT_TYPES...>();
      return std::get<type_id>(type_sets)[in_def.GetIndex()];
    }
  };

  template <typename... TRAIT_TYPES>
  class TraitManager {
    friend class TraitSet<TRAIT_TYPES...>;
  private:
    // A group of trait definitions must be created for each type handled.
    std::tuple< std::vector< TraitDef<TRAIT_TYPES> >... > trait_groups;
    int num_traits;

    static const int num_types = sizeof...(TRAIT_TYPES);

    // Helper Functions:

    // Return a constant indicating the position of a given type in the tuple.
    template <typename BASE_TYPE>
    constexpr static int GetTraitID() {
      return emp::get_type_index<BASE_TYPE, TRAIT_TYPES...>();
    }

    // Return the vector of traits for the given type.
    template <typename IN_TYPE>
    std::vector< TraitDef<IN_TYPE> > & GetTraitGroup() {
      return std::get< GetTraitID<IN_TYPE>() >(trait_groups);
    }

    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    void SetDefaultsByType(TraitSet<TRAIT_TYPES...> & trait_set) const {
      // Get the relevant vectors for the current type.
      std::vector< TraitDef<FIRST_TYPE> > & cur_group = GetTraitGroup<FIRST_TYPE>();
      std::vector<FIRST_TYPE> & type_set = trait_set.template GetTypeSet<FIRST_TYPE>();

      // Set all of the values in type_set
      type_set.resize(0);
      for (TraitDef<FIRST_TYPE> & cur_def : cur_group) {
        type_set.push_back(cur_group.GetDefault());
      }

      // And recurse through the other types.
      SetDefaultsByType<OTHER_TYPES...>(trait_set);
    }

    // template <>
    void SetDefaultsByType(TraitSet<TRAIT_TYPES...> & trait_set) const {
      return;
    }

  public:
    TraitManager() : num_traits(0) { ; }
    ~TraitManager() { ; }

    static int GetNumTypes() { return num_types; }

    int GetNumTraits() const { return num_traits; }
    template <typename IN_TYPE> int GetNumTraitsOfType() const {
      return (int) GetTraitGroup<IN_TYPE>().size();
    }

    // Lookup a trait by its type and index.
    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & GetTrait(int index) {
      std::vector< TraitDef<IN_TYPE> > & cur_group = GetTraitGroup<IN_TYPE>();      
      emp_assert(index >= 0 && index < (int) cur_group.size());
      return cur_group[index];
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

};

#endif
