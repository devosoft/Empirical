#ifndef EMP_TRAIT_H
#define EMP_TRAIT_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  The Trait class maintains a category of measuments about another class.
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
  class Trait {
  private:
    const std::string name;
    const std::string desc;
    const TRAIT_TYPE default_val;

  public:
    Trait(const std::string & _name, const std::string & _desc, const TRAIT_TYPE & _default_val)
      : name(_name), desc(_desc), default_val(_default_val)
    { ; }
    ~Trait() { ; }

    const std::string & GetName() { return name; }
    const std::string & GetDesc() { return desc; }
    const TRAIT_TYPE & GetDefault() { return default_val; }
  };

  template <typename... TRAIT_TYPES>
  class TraitManager {
  private:
    std::tuple< std::vector< Trait<TRAIT_TYPES> >... > trait_sets;
    static const int num_types = sizeof...(TRAIT_TYPES);

    // Helper Functions:

    // Return a constant indicating the position of a given type in the tuple.
    template <typename BASE_TYPE>
    constexpr static int GetTraitID() {
      return emp::get_type_index<BASE_TYPE, TRAIT_TYPES...>();
    }

    // Return the vector of traits for the given type.
    template <typename IN_TYPE>
    std::vector< Trait<IN_TYPE> > & GetTraitSet() {
      return std::get< GetTraitID<IN_TYPE>() >(trait_sets);
    }
  public:
    TraitManager() { ; }
    ~TraitManager() { ; }

    template <typename IN_TYPE>
    void AddTrait(const std::string & _name, const std::string & _desc, const IN_TYPE & _default_val) {
      std::vector< Trait<IN_TYPE> > & cur_set = GetTraitSet<IN_TYPE>();
      cur_set.push_back( Trait<IN_TYPE>(_name, _desc, _default_val) );
      std::cout << cur_set.size() << std::endl;
    }

    static int GetNumTypes() { return num_types; }
  };
};

#endif
