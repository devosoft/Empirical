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

    const std::string & GetName() { return name; }
    const std::string & GetDesc() { return desc; }
    const TRAIT_TYPE & GetDefault() { return default_val; }
  };

  template <typename... TRAIT_TYPES>
  class TraitSet {
  private:
  public:
  };

  template <typename... TRAIT_TYPES>
  class TraitManager {
  private:
    std::tuple< std::vector< TraitDef<TRAIT_TYPES> >... > trait_sets;
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
    std::vector< TraitDef<IN_TYPE> > & GetTraitSet() {
      return std::get< GetTraitID<IN_TYPE>() >(trait_sets);
    }
  public:
    TraitManager() : num_traits(0) { ; }
    ~TraitManager() { ; }

    static int GetNumTypes() { return num_types; }

    int GetNumTraits() const { return num_traits; }
    template <typename IN_TYPE> int GetNumTraitsOfType() const {
      return (int) GetTraitSet<IN_TYPE>().size();
    }

    // Lookup a trait by its type and index.
    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & GetTrait(int index) {
      std::vector< TraitDef<IN_TYPE> > & cur_set = GetTraitSet<IN_TYPE>();      
      emp_assert(index >= 0 && index < (int) cur_set.size());
      return cur_set[index];
    }

    template <typename IN_TYPE>
    const TraitDef<IN_TYPE> & AddTrait(const std::string & _name, const std::string & _desc,
                              const IN_TYPE & _default_val) {
      std::vector< TraitDef<IN_TYPE> > & cur_set = GetTraitSet<IN_TYPE>();
      const int trait_index = (int) cur_set.size();
      cur_set.push_back( TraitDef<IN_TYPE>(_name, _desc, _default_val, trait_index) );
      num_traits++;
      return cur_set[trait_index];
    }

  };
};

#endif
