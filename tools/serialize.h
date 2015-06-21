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
#define EMP_SETUP_SERIALIZE_BASEINFO(CLASS_NAME, BASE_LOAD, BASE_STORE, ...) \
  void EMP_Store(emp::serialize::TextIO & io) {                         \
    BASE_STORE;                                                         \
    emp::serialize::Store(io, __VA_ARGS__);                             \
  }                                                                     \
  explicit CLASS_NAME(emp::serialize::TextIO & io) BASE_LOAD {          \
    emp::serialize::Load(io, __VA_ARGS__);                              \
  }

// Version to use in stand-along classes.
#define EMP_SETUP_SERIALIZE(CLASS_NAME, ...) \
  EMP_SETUP_SERIALIZE_BASEINFO(CLASS_NAME, , , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_SERIALIZE_D(CLASS_NAME, BASE_CLASS, ...)              \
  EMP_SETUP_SERIALIZE_BASEINFO(CLASS_NAME,                              \
                               :BASE_CLASS(io),                         \
                               BASE_CLASS::EMP_Store(io),               \
                               __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).
#define EMP_COMMA_MERGE(A,B) A,B

#define EMP_SETUP_SERIALIZE_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...)   \
  EMP_SETUP_SERIALIZE_BASEINFO(CLASS_NAME,                                  \
                               EMP_COMMA_MERGE(:BASE_CLASS1(io), BASE_CLASS2(io)),     \
                               BASE_CLASS1::EMP_Store(io); BASE_CLASS2::EMP_Store(io), \
                               __VA_ARGS__)
#undef EMP_COMMA

// If there's a base class, 

namespace emp {
namespace serialize {

  struct TextIO {
    std::ostream & os;
    std::istream & is;

    TextIO(std::ostream & _os, std::istream & _is) : os(_os), is(_is) { ; }
    TextIO(std::iostream & _ios) : os(_ios), is(_ios) { ; }

    // Make sure these are never accidentally created or copied.
    TextIO() = delete;
    TextIO(const TextIO &) = delete;
  };
  
  template <typename T>
  void StoreVar(TextIO & IO, T & var) {
    // @CAO for now use ':', but more generally we need to ensure uniquness.
    IO.os << var << ':';
    emp_assert(IO.os);
  }
  
  template <typename T>
  void LoadVar(TextIO & IO, T & var) {
    IO.is >> var;
    IO.is.ignore(1);  // Ignore ':'
    emp_assert(IO.is);
  }
  
  template <>
  void LoadVar<std::string>(TextIO & IO, std::string & var) {
    std::getline(IO.is, var,':');
    emp_assert(IO.is);
  }


  namespace internal {

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;  
  
    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void Store(TextIO & io, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVar(io, arg1);
        serial_impl<OTHER_TYPES...>::Store(io, others...);
      }
      static void Load(TextIO & io, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        LoadVar(io, arg1);
        serial_impl<OTHER_TYPES...>::Load(io, others...);
      }
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void Store(TextIO &) { ; }
      static void Load(TextIO &) { ; }
    };
  };
  
  template <typename... ARG_TYPES>
  void Store(TextIO & io, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Store(io, args...);
  }
  
  template <typename... ARG_TYPES>
  void Load(TextIO & io, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Load(io, args...);
  }
  
};
};

#endif
