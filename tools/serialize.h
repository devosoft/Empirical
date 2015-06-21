#ifndef EMP_SERIALIZE_H
#define EMP_SERIALIZE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to save and load data from classes.
//
//  All of the important information about a class is stored in a CPod, which can be
//  used to restore the class at a later time.
//
//  In order to setup a target class to be able to be serialized into a pod, you must
//  add a macro to include the needed functionality.  For a basic class, use:
//
//   EMP_SETUP_CPOD(ClassName, var1, var2, ...)
//
//  Where ClassName is the target class' name and var1, var2, etc are the names of the
//  member variables that also need to be stored.  Note that member variables can either
//  be either built-in types or custom types that have also had CPods setup in them.
//
//  If the target class is a derived class, you must use either:
//
//   EMP_SETUP_CPOD_D(ClassName, BassClassName, var1, var2, ...)
//
//     -or-
//
//   EMP_SETUP_CPOD_D2(ClassName, BassClass1Name, BaseClass2Name, var1, var2, ...)
//
//  Depending on how many base classes it was derived from (currently max 2).
//
//  Note also that this macro must either go in the public section of the target class
//  definition, or the target class must be made a friend to the emp::serialize::CPod
//  class.
//
//
//  Development Notes:
//  * Use SFINAE technique to know when classes have an internal save already setup.
//  * Come up with a better name for an encoding than TextIO.  Pod? Seed? CPod
//  * To deal with pointers we should recurse, but keep map to new pointer locations.
//  * Add a mechanism to set value to constant rather than previous value.
//

#include <iostream>

// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_CPOD_BASEINFO(CLASS_NAME, BASE_LOAD, BASE_STORE, ...) \
  void EMP_Store(emp::serialize::CPod & io) {                           \
    BASE_STORE;                                                         \
    emp::serialize::Store(io, __VA_ARGS__);                             \
  }                                                                     \
  explicit CLASS_NAME(emp::serialize::CPod & io) BASE_LOAD {            \
    emp::serialize::Load(io, __VA_ARGS__);                              \
  }

// Version to use in stand-along classes.
#define EMP_SETUP_CPOD(CLASS_NAME, ...) \
  EMP_SETUP_CPOD_BASEINFO(CLASS_NAME, , , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_CPOD_D(CLASS_NAME, BASE_CLASS, ...)                   \
  EMP_SETUP_CPOD_BASEINFO(CLASS_NAME,                                   \
                               :BASE_CLASS(io),                         \
                               BASE_CLASS::EMP_Store(io),               \
                               __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).
#define EMP_COMMA_MERGE(A,B) A,B

#define EMP_SETUP_CPOD_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...)    \
  EMP_SETUP_CPOD_BASEINFO(CLASS_NAME,                                   \
                EMP_COMMA_MERGE(:BASE_CLASS1(io), BASE_CLASS2(io)),     \
                BASE_CLASS1::EMP_Store(io); BASE_CLASS2::EMP_Store(io), \
                __VA_ARGS__)
#undef EMP_COMMA

// If there's a base class, 

namespace emp {
namespace serialize {

  struct CPod {
    std::ostream & os;
    std::istream & is;

    CPod(std::ostream & _os, std::istream & _is) : os(_os), is(_is) { ; }
    CPod(std::iostream & _ios) : os(_ios), is(_ios) { ; }
    cPod() 

    // Make sure these are never accidentally created or copied.
    CPod() = delete;
    CPod(const CPod &) = delete;
  };
  
  template <typename T>
  void StoreVar(CPod & IO, T & var) {
    // @CAO for now use ':', but more generally we need to ensure uniquness.
    IO.os << var << ':';
    emp_assert(IO.os);
  }
  
  template <typename T>
  void LoadVar(CPod & IO, T & var) {
    IO.is >> var;
    IO.is.ignore(1);  // Ignore ':'
    emp_assert(IO.is);
  }
  
  template <>
  void LoadVar<std::string>(CPod & IO, std::string & var) {
    std::getline(IO.is, var,':');
    emp_assert(IO.is);
  }


  namespace internal {

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;  
  
    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void Store(CPod & io, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVar(io, arg1);
        serial_impl<OTHER_TYPES...>::Store(io, others...);
      }
      static void Load(CPod & io, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        LoadVar(io, arg1);
        serial_impl<OTHER_TYPES...>::Load(io, others...);
      }
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void Store(CPod &) { ; }
      static void Load(CPod &) { ; }
    };
  };
  
  template <typename... ARG_TYPES>
  void Store(CPod & io, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Store(io, args...);
  }
  
  template <typename... ARG_TYPES>
  void Load(CPod & io, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Load(io, args...);
  }
  
};
};

#endif
