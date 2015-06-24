#ifndef EMP_SERIALIZE_H
#define EMP_SERIALIZE_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Tools to save and load data from classes.
//
//  All of the important information about a class is stored in a DataPod, which can be
//  used to restore the class at a later time.
//
//  In order to setup a target class to be able to be serialized into a pod, you must
//  add a macro to include the needed functionality.  For a basic class, use:
//
//   EMP_SETUP_DATAPOD(ClassName, var1, var2, ...)
//
//  Where ClassName is the target class' name and var1, var2, etc are the names of the
//  member variables that also need to be stored.  Note that member variables can either
//  be either built-in types or custom types that have also had DataPods setup in them.
//
//  If the target class is a derived class, you must use either:
//
//   EMP_SETUP_DATAPOD_D(ClassName, BassClassName, var1, var2, ...)
//
//     -or-
//
//   EMP_SETUP_DATAPOD_D2(ClassName, BassClass1Name, BaseClass2Name, var1, var2, ...)
//
//  Depending on how many base classes it was derived from (currently max 2).
//
//  Note also that this macro must either go in the public section of the target class
//  definition, or the target class must be made a friend to the emp::serialize::DataPod
//  class.
//
//
//  Development Notes:
//  * Constructors should call correct internal member constructors, not rely on assignment
//  * Use SFINAE technique to know when classes have an internal save already setup.
//  * To deal with pointers we should recurse, but keep map to new pointer locations.
//  * Add a mechanism to set value to constant rather than previous value.
//

#include <iostream>

#include "macros.h"

// Use this macro to automatically build methods in a class to save and load data.
#define EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, BASE_LOAD, BASE_STORE, ...) \
  void EMP_Store(emp::serialize::DataPod & pod) {                       \
    BASE_STORE;                                                         \
    emp::serialize::Store(pod, __VA_ARGS__);                            \
  }                                                                     \
  explicit CLASS_NAME(emp::serialize::DataPod & pod) BASE_LOAD {        \
    emp::serialize::Load(pod, __VA_ARGS__);                             \
  }

// Version to use in stand-along classes.
#define EMP_SETUP_DATAPOD(CLASS_NAME, ...) \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME, , , __VA_ARGS__)

// Version to use in derived classes (with a base that also needs to be serialized).
#define EMP_SETUP_DATAPOD_D(CLASS_NAME, BASE_CLASS, ...)    \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                    \
                          :BASE_CLASS(pod),              \
                          BASE_CLASS::EMP_Store(pod),    \
                          __VA_ARGS__)

// Version to use in derived classes (with TWO bases that need to be serialized).

#define EMP_SETUP_DATAPOD_D2(CLASS_NAME, BASE_CLASS1, BASE_CLASS2, ...)    \
  EMP_SETUP_DATAPOD_BASEINFO(CLASS_NAME,                                   \
                 EMP_COMMA_MERGE(:BASE_CLASS1(pod), BASE_CLASS2(pod)),     \
                 BASE_CLASS1::EMP_Store(pod); BASE_CLASS2::EMP_Store(pod), \
                __VA_ARGS__)


// If there's a base class, 

namespace emp {
namespace serialize {

  class DataPod {
  protected:
    std::ostream * os;
    std::istream * is;
    bool own_os;
    bool own_is;

    void ClearData() { 
      if (own_os && os) delete os;
      if (own_is && is) delete is;
    }
  public:
    DataPod(std::ostream & _os, std::istream & _is)
      : os(&_os), is(&_is), own_os(false), own_is(false) { ; }
    DataPod(std::iostream & _ios) : DataPod(_ios, _ios) { ; }

    // Allow move transfer of a DataPod.
    DataPod(DataPod && rhs) : os(rhs.os), is(rhs.is), own_os(rhs.own_os), own_is(rhs.own_is) {
      rhs.own_os = false; rhs.own_is=false;
    }
    DataPod & operator=(DataPod && rhs) {
      ClearData();
      os = rhs.os;  is = rhs.is;  own_os = rhs.own_os;  own_is = rhs.own_is;
      rhs.own_os = false; rhs.own_is=false;
      return *this;
    }

    // Make sure these are never accidentally created or copied.
    DataPod() = delete;
    DataPod(const DataPod &) = delete;

    ~DataPod() { ClearData(); }

    std::ostream & OStream() { return *os; }
    std::istream & IStream() { return *is; }
  };
  
  template <typename T>
  void StoreVar(DataPod & pod, T & var) {
    // @CAO for now use ':', but more generally we need to ensure uniquness.
    pod.OStream() << var << ':';
    emp_assert(pod.OStream());
  }
  
  template <typename T>
  void LoadVar(DataPod & pod, T & var) {
    pod.IStream() >> var;
    pod.IStream().ignore(1);  // Ignore ':'
    emp_assert(pod.IStream());
  }
  
  template <>
  void LoadVar<std::string>(DataPod & pod, std::string & var) {
    std::getline(pod.IStream(), var,':');
    emp_assert(pod.IStream());
  }


  namespace internal {

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;  
  
    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void Store(DataPod & pod, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVar(pod, arg1);
        serial_impl<OTHER_TYPES...>::Store(pod, others...);
      }
      static void Load(DataPod & pod, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        LoadVar(pod, arg1);
        serial_impl<OTHER_TYPES...>::Load(pod, others...);
      }
    };
    
    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void Store(DataPod &) { ; }
      static void Load(DataPod &) { ; }
    };
  };
  
  template <typename... ARG_TYPES>
  void Store(DataPod & pod, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Store(pod, args...);
  }
  
  template <typename... ARG_TYPES>
  void Load(DataPod & pod, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Load(pod, args...);
  }
  
};
};

#endif
