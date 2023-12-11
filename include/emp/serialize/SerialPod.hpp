/**
 * @note This file is part of Empirical, https://github.com/devosoft/Empirical
 * @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 * @date 2023.
 *
 * @file SerialPod.hpp
 * @brief Tools to save and load data from classes.
 * @note Status: ALPHA
 *
 * The SerialPod object knows whether it is loading or saving and object, and provides a
 * simple mechanism to do so.
 * 
 * How to use:
 *  A standard class can use a SerialPod by adding a `Serialize(SerialPod & pod)` member function.
 *
 *  An object of an independent class MyClass with a fixed interface can have a stand-alone
 *  Serialize function in the form of `Serialize(SerialPod & pos, const MyClass & obj)`.
 *
 *  Any class can have a constructor that takes a `SerialPod` to allow reconstruction of const
 *  objects.  As long as such a constructor is provided, the Serialize function can be const.
 * 
 *  More complex classes (e.g., those that do memory management) will need to have seprate
 *  `SerialSave(SerialPod & pos) const` and `SerialLoad(SerialPod & pos)` functions.
 *
 * Example:
 *  Inside of MyClass we might have:
 * 
 *  void MyClass(SerialPod & pod) {
 *    pod.Serialize(member_a, member_b, member_c);  // List all members to save/load
 *  }
 */

#ifndef EMP_SERIALIZE_SERIALPOD_HPP_INCLUDE
#define EMP_SERIALIZE_SERIALPOD_HPP_INCLUDE

#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"

namespace emp {

  /// A SerialPod manages information about other classes for serialization.
  class SerialPod {
  private:
    using is_ptr = emp::Ptr<std::istream>;
    using os_ptr = emp::Ptr<std::ostream>;
    std::variant<is_ptr, os_ptr> stream_ptr = static_cast<is_ptr>(nullptr);
    bool own_ptr = false;

    void ClearData() {
      // If we own this pointer, delete it.
      if (own_ptr) {
        std::visit([](auto ptr) { ptr.Delete(); }, stream_ptr);
        own_ptr = false;
      }
      stream_ptr = static_cast<is_ptr>(nullptr);
    }

  public:
    SerialPod(std::ostream & os) : stream_ptr(emp::ToPtr(&os)) { ; }
    SerialPod(std::istream & is) : stream_ptr(emp::ToPtr(&is)) { ; }
    SerialPod(std::iostream & ios, bool is_save) {
      if (is_save) stream_ptr = emp::ToPtr<std::ostream>(&ios);
      else stream_ptr = emp::ToPtr<std::istream>(&ios);
    }

    /// Move constructor
    SerialPod(SerialPod && rhs) : stream_ptr(rhs.stream_ptr), own_ptr(rhs.own_ptr) {
      rhs.own_ptr = false;
    }

    /// Move operator
    SerialPod & operator=(SerialPod && rhs) {
      ClearData();
      stream_ptr = rhs.stream_ptr;
      own_ptr = rhs.own_ptr;
      rhs.own_ptr = false;
      return *this;
    }

    // Make sure these are never accidentally created or copied.
    SerialPod() = delete;
    SerialPod(const SerialPod &) = delete;

    ~SerialPod() { ClearData(); }

    bool IsLoad() const { return std::holds_alternative<is_ptr>(stream_ptr); }
    bool IsSave() const { return std::holds_alternative<os_ptr>(stream_ptr); }
  };


  /// StoreVar() takes a SerialPod and a variable and stores that variable to the pod.
  /// The third argument (bool vs. int) will receive a bool, and thus bool versions
  /// are preferred in the case of a tie.  Specialized versions of this function can be
  /// included elsewhere, as needed, and should take a bool as the third argument.

  // Custom classes should run their built-in EMP_Store member function.
  template <typename T>
  auto StoreVar(SerialPod & pod, const T & var, bool) -> typename T::emp_load_return_type & {
    var.EMP_Store(pod);
    return pod;
  }

  // Special standard library types need to have a custom StoreVar method built.
  template <typename T>
  void StoreVar(SerialPod & pod, const emp::vector<T> & var, bool) {
    StoreVar(pod, var.size(), true);
    for (int i = 0; i < (int) var.size(); ++i) {
      StoreVar(pod, var[i], true);
    }

    // @CAO for now use ':' separator, but more generally we need to ensure uniquness.
    pod.OStream() << ':';
    // emp_assert(pod.OStream()); // @ELD - this is throwing a compiler error
  }

  // As a fallback, just send the saved object to the SerialPod's output stream.
  template <typename T>
  void StoreVar(SerialPod & pod, const T & var, int) {
    // @CAO for now use ':', but more generally we need to ensure uniquness.
    pod.OStream() << var << ':';
    // emp_assert(pod.OStream()); // @ELD - this is throwing a compiler error
  }


  // The SetupLoad() function determines what type of information a constructor needs from
  // a SerialPod (based on the objects types) and returns that information.  By default, the
  // function will produce an instance of the needed type to trigger the copy constructor,
  // but if a constructor exists that takes a SerialPod it will use that instead.

  // Use SFINAE technique to identify custom types.
  template <typename T>
  auto SetupLoad(SerialPod & pod, T*, bool) -> typename T::emp_load_return_type & {
    return pod;
  }

  // Otherwise use default streams.
  template <typename T>
  auto SetupLoad(SerialPod & pod, const T*, int) -> T {
    // std::decay<T> var;
    T var;
    pod.IStream() >> var;
    pod.IStream().ignore(1);  // Ignore ':'
    emp_assert(pod.IStream() && "Make sure the SerialPod is still okay.");
    return var;
  }

  // Use special load for strings.
  std::string SetupLoad(SerialPod & pod, std::string *, bool) {
    std::string var;
    std::getline(pod.IStream(), var, ':');
    emp_assert(pod.IStream() && "Make sure the SerialPod is still okay.");
    return var;
  }

  // Use special load for vectors of arbitrary type.
  template <typename T>
  emp::vector<T> SetupLoad(SerialPod & pod, emp::vector<T> *, bool) {
    const uint32_t v_size(SetupLoad(pod, &v_size, true));
    emp::vector<T> var;

    // Create vector of correct size and create elements with pod.
    for (uint32_t i = 0; i < v_size; i++) {
      var.emplace_back(SetupLoad(pod, &(var[0]), true));
    }
    emp_assert(pod.IStream() && "Make sure the SerialPod is still okay.");
    return var;
  }



  // Setup for a variadic Store() function that systematically save all variables in a SerialPod.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {

    // Base implementaion to specialize on.
    template <typename... IGNORE> struct serial_impl;

    // Specialized to recurse, storing or loading individual vars.
    template <typename FIRST_TYPE, typename... OTHER_TYPES>
    struct serial_impl<FIRST_TYPE, OTHER_TYPES...> {
      static void Store(SerialPod & pod, FIRST_TYPE & arg1, OTHER_TYPES&... others) {
        StoreVar(pod, arg1, true);
        serial_impl<OTHER_TYPES...>::Store(pod, others...);
      }
    };

    // End condition for recursion when no vars remaining.
    template <> struct serial_impl<> {
      static void Store(SerialPod &) { ; }
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  template <typename... ARG_TYPES>
  void Store(SerialPod & pod, ARG_TYPES&... args) {
    internal::serial_impl<ARG_TYPES...>::Store(pod, args...);
  }

}
}

#endif // #ifndef EMP_IO_SERIALIZE_HPP_INCLUDE
