#ifndef EMP_SERIALIZE_H
#define EMP_SERIALIZE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to save and load data from classes.
//
//  Development Notes:
//  * Macro should setup a constructor that takes TextIO as an argument to build class.
//  * Need second macro for derived classes so base class can be called recursively.
//  * To deal with pointers we should recurse, but keep map to new pointer locations.
//  * Add a mechanism to set value to constant rather than previous value.
//

#include <iostream>

// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_SERIALIZE_INTERNAL(CLASS_NAME, BASE_INFO, ...) \
  void EMP_StoreAsText(std::ostream & os) {        \
    emp::serialize::StoreAsText(os, __VA_ARGS__);  \
  }                                              \
  CLASS_NAME(std::istream & is) BASE_INFO {      \
    emp::serialize::LoadAsText(is, __VA_ARGS__);   \
  }

// Version to use in stand-along classes.
#define EMP_SETUP_SERIALIZE(CLASS_NAME, ...) \
  EMP_SETUP_SERIALIZE_INTERNAL(CLASS_NAME, , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_SERIALIZE_D(CLASS_NAME, BASE_CLASS, ...)         \
  EMP_SETUP_SERIALIZE_INTERNAL(CLASS_NAME, :BASE_CLASS(is), __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).
// @CAO NOTE: Right not BASE_CLASS2 is ignored!!!
#define EMP_COMMA ,
#define EMP_SETUP_SERIALIZE_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...) \
  EMP_SETUP_SERIALIZE_INTERNAL(CLASS_NAME, :BASE_CLASS1(is) EMP_COMMA BASE_CLASS2(is), __VA_ARGS__)
#undef EMP_COMMA

// If there's a base class, 

namespace emp {
namespace serialize {

  using TextIO = std::stringstream;

  namespace internal {

    template <typename T>
    void StoreVarAsText(std::ostream & os, T & var) {
      // @CAO for now use ':', but more generally we need to ensure uniquness.
      os << var << ':';
      emp_assert(os);
    }

    template <typename T>
    void LoadVarAsText(std::istream & is, T & var) {
      is >> var;
      std::cout << "Loaded '" << var << "'" << std::endl;
      emp_assert(is);
    }

    template <>
    void LoadVarAsText<std::string>(std::istream & is, std::string & var) {
      std::getline(is, var,':');
      std::cout << "Loaded '" << var << "'" << std::endl;
      emp_assert(is);
    }

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;  
  
    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void StoreAsText(std::ostream & os, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVarAsText(os, arg1);
        serial_impl<OTHER_TYPES...>::StoreAsText(os, others...);
      }
      static void LoadAsText(std::istream & is, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        LoadVarAsText(is, arg1);
        is.ignore(1);  // Ignore ':'
        emp_assert(is);
        serial_impl<OTHER_TYPES...>::LoadAsText(is, others...);
      }
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void StoreAsText(std::ostream &) { ; }
      static void LoadAsText(std::istream &) { ; }
    };
  };
  
  template <typename... ARG_TYPES>
  void StoreAsText(std::ostream & os, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::StoreAsText(os, args...);
  }
  
  template <typename... ARG_TYPES>
  void LoadAsText(std::istream & is, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::LoadAsText(is, args...);
  }
  
};
};

#endif
