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
#define EMP_SETUP_SERIALIZE(CLASS_NAME, ...)     \
  void EMP_StoreText(std::ostream & os) {        \
    emp::serialize::StoreText(os, __VA_ARGS__);  \
  }                                              \
  CLASS_NAME(std::istream & is) {         \
    emp::serialize::LoadText(is, __VA_ARGS__);   \
  }


namespace emp {
namespace serialize {

  using TextIO = std::stringstream;

  namespace internal {
    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;  
  
    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void StoreText(std::ostream & os, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        os << ':' << arg1;
        serial_impl<OTHER_TYPES...>::StoreText(os, others...);
      }
      static void LoadText(std::istream & is, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        char tmp_char;
        is >> tmp_char >> arg1;
        // std::cout << "[" << tmp_char << "]" << std::endl;
        serial_impl<OTHER_TYPES...>::LoadText(is, others...);
      }
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void StoreText(std::ostream &) { ; }
      static void LoadText(std::istream &) { ; }
    };
  };
  
  template <typename... ARG_TYPES>
  void StoreText(std::ostream & os, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::StoreText(os, args...);
  }
  
  template <typename... ARG_TYPES>
  void LoadText(std::istream & is, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::LoadText(is, args...);
  }
  
};
};

#endif
