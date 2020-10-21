/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2019
 *
 *  @file serialize.hpp
 *  @brief Tools to save and load data from classes.
 *  @note Status: ALPHA
 *
 *  All of the important information about a class is stored in a DataPod, which can be
 *  used to restore the class at a later time.
 *
 *  Why is this better than other serialization techniques?
 *  1. Only one line of code is added to a custom class to make it serializable.
 *  2. Serialized objects do not need a default constructor (a DataPod constructor is added)
 *  3. Serialized objects can be const since they get rebuilt during construction.
 *  4. Synergistic interactions with other EMP classes, such as config and tuple_struct
 *
 *  In order to setup a target class to be able to be serialized into a pod, you must
 *  add a macro to include the needed functionality.  For a basic class, use:
 *
 *   EMP_SETUP_DATAPOD(ClassName, var1, var2, ...)
 *
 *  Where ClassName is the target class' name and var1, var2, etc are the names of the
 *  member variables that also need to be stored.  Note that member variables can either
 *  be either built-in types or custom types that have also had DataPods setup in them.
 *
 *  If the target class is a derived class, you must use either:
 *
 *   EMP_SETUP_DATAPOD_D(ClassName, BassClassName, var1, var2, ...)
 *
 *     -or-
 *
 *   EMP_SETUP_DATAPOD_D2(ClassName, BassClass1Name, BaseClass2Name, var1, var2, ...)
 *
 *  ...depending on how many base classes it was derived from (currently max 2).
 *
 *  Note also that this macro must either go in the public section of the target class
 *  definition, or the target class must be made a friend to the emp::serialize::DataPod
 *  class.
 *
 *
 *  @todo Build custom load/store function for more STL objects (especially containers)
 *  @todo To deal with pointers we should recurse, but keep map to new pointer locations.
 *  @todo Setup a more robust method for dealing with arbitrary strings so we don't have
 *        to worry about collisions in streams (JSon format??)
 *  @todo Setup a (compressed) binary save formmat in DataPods in addition to JSon.
 *  @todo Setup promised synergistic interactions with config and tuple_struct to auto
 *        store and load without any additional effort on the part of the library user.
 */

#ifndef EMP_SERIALIZE_H
#define EMP_SERIALIZE_H

#include <iostream>

#include "../base/vector.hpp"
#include "serialize_macros.hpp"

namespace emp {
namespace serialize {

  /// A DataPod managed information about another class for serialization.
  class DataPod {
  protected:
    std::ostream * os;
    std::istream * is;
    bool own_os = false;
    bool own_is = false;

    void ClearData() {
      if (own_os && os) delete os;
      if (own_is && is) delete is;
    }
  public:
    DataPod(std::ostream & _os, std::istream & _is) : os(&_os), is(&_is) { ; }
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


  /// StoreVar() takes a DataPod and a variable and stores that variable to the pod.
  /// The third argument (bool vs. int) will receive a bool, and thus bool versions
  /// are preferred in the case of a tie.  Specialized versions of this function can be
  /// included elsewhere, as needed, and should take a bool as the third argument.

  // Custom classes should run their built-in EMP_Store member function.
  template <typename T>
  auto StoreVar(DataPod & pod, const T & var, bool) -> typename T::emp_load_return_type & {
    var.EMP_Store(pod);
    return pod;
  }

  // Special standard library types need to have a custom StoreVar method built.
  template <typename T>
  void StoreVar(DataPod & pod, const emp::vector<T> & var, bool) {
    StoreVar(pod, var.size(), true);
    for (int i = 0; i < (int) var.size(); ++i) {
      StoreVar(pod, var[i], true);
    }

    // @CAO for now use ':' separator, but more generally we need to ensure uniquness.
    pod.OStream() << ':';
    // emp_assert(pod.OStream()); // @ELD - this is throwing a compiler error
  }

  // As a fallback, just send the saved object to the DataPod's output stream.
  template <typename T>
  void StoreVar(DataPod & pod, const T & var, int) {
    // @CAO for now use ':', but more generally we need to ensure uniquness.
    pod.OStream() << var << ':';
    // emp_assert(pod.OStream()); // @ELD - this is throwing a compiler error
  }


  // The SetupLoad() function determines what type of information a constructor needs from
  // a DataPod (based on the objects types) and returns that information.  By default, the
  // function will produce an instance of the needed type to trigger the copy constructor,
  // but if a constructor exists that takes a DataPod it will use that instead.

  // Use SFINAE technique to identify custom types.
  template <typename T>
  auto SetupLoad(DataPod & pod, T*, bool) -> typename T::emp_load_return_type & {
    return pod;
  }

  // Otherwise use default streams.
  template <typename T>
  auto SetupLoad(DataPod & pod, const T*, int) -> T {
    // std::decay<T> var;
    T var;
    pod.IStream() >> var;
    pod.IStream().ignore(1);  // Ignore ':'
    emp_assert(pod.IStream() && "Make sure the DataPod is still okay.");
    return var;
  }

  // Use special load for strings.
  std::string SetupLoad(DataPod & pod, std::string *, bool) {
    std::string var;
    std::getline(pod.IStream(), var, ':');
    emp_assert(pod.IStream() && "Make sure the DataPod is still okay.");
    return var;
  }

  // Use special load for vectors of arbitrary type.
  template <typename T>
  emp::vector<T> SetupLoad(DataPod & pod, emp::vector<T> *, bool) {
    const uint32_t v_size(SetupLoad(pod, &v_size, true));
    emp::vector<T> var;

    // Create vector of correct size and create elements with pod.
    for (uint32_t i = 0; i < v_size; i++) {
      var.emplace_back(SetupLoad(pod, &(var[0]), true));
    }
    emp_assert(pod.IStream() && "Make sure the DataPod is still okay.");
    return var;
  }



  // Setup for a variadic Store() function that systematically save all variables in a DataPod.

  namespace internal {

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;

    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void Store(DataPod & pod, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVar(pod, arg1, true);
        serial_impl<OTHER_TYPES...>::Store(pod, others...);
      }
    };

    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void Store(DataPod &) { ; }
    };
  }

  template <typename... ARG_TYPES>
  void Store(DataPod & pod, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Store(pod, args...);
  }

}
}

#endif
