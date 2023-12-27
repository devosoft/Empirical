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
 * => How to use:
 *  A standard class can use a SerialPod by adding a `Serialize(SerialPod & pod)` member function.
 *
 *  An object of an independent class MyClass with a fixed interface can have a stand-alone
 *  Serialize function in the form of `Serialize(SerialPod & pod, const MyClass & obj)`.
 *
 *  Any class can have a constructor that takes a `SerialPod` to allow reconstruction of const
 *  objects.  As long as such a constructor is provided, the Serialize function can be const.
 * 
 *  More complex classes (e.g., those that do memory management) will need to have seprate
 *  `SerialSave(SerialPod & pos) const` and `SerialLoad(SerialPod & pos)` functions.
 *
 * => Example:
 *  Inside of MyClass we might have:
 * 
 *  void MyClass(SerialPod & pod) {
 *    pod(member_a, member_b, member_c);  // List all members to save/load
 *  }
 * 
 * => Resolving duplications:
 *  If more than one version of a serialization function exists, functions external to the class
 *  always have precedence over those internal (so that classes can later be changed how they
 *  serialize).
 * 
 *  If both a specific function (e.g., Load) and general function (i.e, Serialize) exist, the
 *  more specific one will always be used.
 * 
 *  Regular streaming is used only when no other options exist.
 */

#ifndef EMP_SERIALIZE_SERIALPOD_HPP_INCLUDE
#define EMP_SERIALIZE_SERIALPOD_HPP_INCLUDE

#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "../base/concepts.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../meta/type_traits.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  // Pre-declarations
  class SerialPod; 
  class String;

  // Base version of functions so that they can be tested in the concepts below...
  template <typename T> void Serialize(SerialPod & pod, std::vector<T> & value);
  template <typename T> void SerialLoad(SerialPod & pod, std::vector<T> & value);
  template <typename T> void SerialSave(SerialPod & pod, std::vector<T> & value);

  /// Concept to identify id a type has a Serialize() member function.
  template <typename OBJ_T>
  concept hasSerializeMember = requires(OBJ_T & value, SerialPod & pod) {
    { value.Serialize(pod) };
  };

  /// Concept to identify id a type has a SerialLoad() member function.
  template <typename OBJ_T>
  concept hasSerialLoadMember = requires(OBJ_T & value, SerialPod & pod) {
    { value.SerialLoad(pod) };
  };

  /// Concept to identify id a type has a SerialSave() member function.
  template <typename OBJ_T>
  concept hasSerialSaveMember = requires(OBJ_T & value, SerialPod & pod) {
    { value.SerialSave(pod) };
  };

  /// Concept to identify id a type has a stand-alone Serialize() overload.
  template <typename OBJ_T>
  concept hasSerializeOverload = requires(OBJ_T & value, SerialPod & pod) {
    { Serialize(pod, value) };
  };

  /// Concept to identify id a type has a stand-alone SerialLoad() overload.
  template <typename OBJ_T>
  concept hasSerialLoadOverload = requires(OBJ_T & value, SerialPod & pod) {
    { SerialLoad(pod, value) };
  };

  /// Concept to identify id a type has a stand-alone SerialSave() overload.
  template <typename OBJ_T>
  concept hasSerialSaveOverload = requires(OBJ_T & value, SerialPod & pod) {
    { SerialSave(pod, value) };
  };

  /// Concept to identify id a type has a de-serialization constructor.
  template <typename OBJ_T>
  concept hasSerializeConstructor = requires(SerialPod & pod) {
    { OBJ_T(pod) };
  };


  /// A SerialPod manages information about other classes for serialization.
  class SerialPod {
  private:
    using is_ptr_t = emp::Ptr<std::istream>;
    using os_ptr_t = emp::Ptr<std::ostream>;
    std::variant<is_ptr_t, os_ptr_t> stream_ptr = static_cast<is_ptr_t>(nullptr);
    bool own_ptr = false;

    std::istream & IStream() { return *std::get<is_ptr_t>(stream_ptr); }
    std::ostream & OStream() { return *std::get<os_ptr_t>(stream_ptr); }

    void ClearData() {
      // If we own this pointer, delete it.
      if (own_ptr) {
        std::visit([](auto ptr) { ptr.Delete(); }, stream_ptr);
        own_ptr = false;
      }
      stream_ptr = static_cast<is_ptr_t>(nullptr);
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

    bool IsLoad() const { return std::holds_alternative<is_ptr_t>(stream_ptr); }
    bool IsSave() const { return std::holds_alternative<os_ptr_t>(stream_ptr); }

    // Load a stand-alone value.
    template <typename T>
    T LoadValue() {
      if constexpr (hasSerializeConstructor<T>) { return T(*this); }
      else {
        T temp;
        Load(temp);
        return temp;
      }
    }

    SerialPod & Load() { return *this; } // Base case...
    SerialPod & Save() { return *this; } // Base case...

    template <typename T, typename... EXTRA_Ts>
    SerialPod & Load(T & in, EXTRA_Ts &... extras) {
      static_assert(!emp::is_ptr_type<std::decay<T>>(),
        "SerialPod cannot load or save pointers without more information.\n"
        "Use ManagePtr(value) for restoring pointers by first building the instance,\n"
        "or LinkPtr(value) to use the value of a pointer that is managed elsewhere." );
      if constexpr (std::is_same<T, std::string>() || std::is_same<T, emp::String>()) {
        std::getline(IStream(), in, '\n');
        in = emp::from_literal_string(in);
      }
      else if constexpr (hasSerialLoadOverload<T>) { SerialLoad(*this, in); }
      else if constexpr (hasSerializeOverload<T>) { Serialize(*this, in); }
      else if constexpr (hasSerialLoadMember<T>) { in.SerialLoad(*this); }
      else if constexpr (hasSerializeMember<T>) { in.Serialize(*this); }
      else if constexpr (std::is_enum<T>()) { // enums must be converted properly.
        std::string str;
        std::getline(IStream(), str, '\n');
        in = static_cast<T>(std::stoi(str));
      }
      else if constexpr (canStreamTo<std::ostream,T> && canStreamFrom<std::istream,T>) {
        std::string str;
        std::getline(IStream(), str, '\n');
        std::stringstream ss(str);
        ss >> in;
      }
      else { notify::Error("Invalid SerialPod::Load attempt."); };
      return Load(extras...);
    }

    template <typename T, typename... EXTRA_Ts>
    SerialPod & Save(T & in, EXTRA_Ts &... extras) {
      static_assert(!emp::is_ptr_type<std::decay<T>>(),
        "SerialPod cannot load or save pointers without more information.\n"
        "Use ManagePtr(value) for restoring pointers by first building the instance,\n"
        "or LinkPtr(value) to use the value of a pointer that is managed elsewhere." );
      if constexpr (std::is_same<T, std::string>() || std::is_same<T, emp::String>()) {
        OStream() << '\"';
        for (char c : in) { OStream() << emp::to_escaped_string(c); }
        OStream() << "\"\n";
      }
      else if constexpr (hasSerialSaveOverload<T>) { SerialSave(*this, in); }
      else if constexpr (hasSerializeOverload<T>) { Serialize(*this, in); }
      else if constexpr (hasSerialSaveMember<T>) { in.SerialSave(*this); }
      else if constexpr (hasSerializeMember<T>) { in.Serialize(*this); }
      else if constexpr (std::is_enum<T>()) { // enums must be converted to numerical values.
        OStream() << static_cast<int>(in) << '\n';
      }
      else if constexpr (canStreamTo<std::ostream,T> && canStreamFrom<std::istream,T>) {
        OStream() << in << '\n';
      }
      else { notify::Error("Invalid SerialPod::Save attempt."); }
      return Save(extras...);
    }

    template <typename T, typename... EXTRA_Ts>
    SerialPod & operator()(T & in, EXTRA_Ts &... extras) {
      if (IsLoad()) return Load(in, extras...);
      else return Save(in, extras...);
    }
  };

  template <typename T>
  void Serialize(SerialPod & pod, std::vector<T> & vec) {
    if (pod.IsSave()) SerialSave(pod, vec);
    else SerialLoad(pod, vec);
  }

  template <typename T>
  void SerialLoad(SerialPod & pod, std::vector<T> & vec) {
    const size_t size = pod.LoadValue<size_t>();

    if constexpr (hasSerializeConstructor<T>) {
      vec.reserve(size);
      for (size_t i=0; i < size; ++i) {
        vec.emplace_back(pod);
      }
    } else {
      vec.resize(size);
      for (auto & element : vec) {
        pod.Load(element);
      }
    }
  }

  template <typename T>
  void SerialSave(SerialPod & pod, std::vector<T> & vec) {
    pod.Save(vec.size());
    for (auto & element : vec) {
      pod.Save(element);
    }
  }


} // Close namespace emp

#endif // #ifndef EMP_IO_SERIALIZE_HPP_INCLUDE
