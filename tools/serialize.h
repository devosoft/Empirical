#ifndef EMP_SERIALIZE_H
#define EMP_SERIALIZE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to save and load data from classes.
//

#include <iostream>


// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_SERIALIZE(...)                 \
  void EMP_StoreText(std::ostream & os) {        \
    emp::serialize::StoreText(os, __VA_ARGS__);  \
  }                                              \
  void EMP_LoadText(std::istream & is) {         \
    emp::serialize::LoadText(is, __VA_ARGS__);   \
  }


namespace emp {
namespace serialize {

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
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> { static void StoreText(std::ostream &) { ; } };
  };
  
  template <typename... ARG_TYPES>
  void StoreText(std::ostream & os, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::StoreText(os, args...);
  }
  
  template <typename... ARG_TYPES>
  void LoadText(std::istream & is, ARG_TYPES&... args) {
    // internal::serial_impl<ARG_TYPES...>::LoadText(is, args...);
  }
  
};
};

#endif
