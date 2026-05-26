/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2023-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/serialize/SerialPod.hpp
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
 *  Serialize function in the form of `Serialize(SerialPod & pod, MyClass & obj)`.
 *
 *  Any class can have a constructor that takes a `SerialPod` to allow reconstruction of const
 *  objects.  As long as such a constructor is provided, the Serialize function can be const.
 *
 *  More complex classes (e.g., those that do memory management) will need to have separate
 *  `SerialSave(SerialPod & pod) const` and `SerialLoad(SerialPod & pod)` functions.
 *
 *  A pod can be used as a functor to continue the serialization process.
 * 
 * => Example:
 *  Inside of MyClass we might have:
 *
 *  MyClass(SerialPod & pod) {
 *    pod(member_a, member_b, member_c);  // Reconstruct all members
 *  }
 *
 *  void Serialize(SerialPod & pod) {
 *    pod(member_a, member_b, member_c);  // List all members to save/load
 *  }
 * 
 * => Disambiguation:
 *  If more than one version of a serialization function exists, external functions have
 *  precedence over internal (so that a user can override how a class is serialized).
 *
 *  If both a specific function (e.g., Load) and general function (i.e, Serialize) exist, the
 *  more specific one will always be used.
 */

#pragma once

#ifndef INCLUDE_EMP_SERIALIZE_SERIAL_POD_HPP_GUARD
#define INCLUDE_EMP_SERIALIZE_SERIAL_POD_HPP_GUARD

#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "../base/concepts.hpp"
#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../meta/type_traits.hpp"
#include "../meta/TypeID.hpp"
#include "../tools/string_utils.hpp"

namespace emp {

  // Pre-declarations
  class SerialPod;
  class String;

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

  /// Concept to identify if a type has a stand-alone SerialLoad() overload.
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
      if (is_save) {
        stream_ptr = emp::ToPtr<std::ostream>(&ios);
      } else {
        stream_ptr = emp::ToPtr<std::istream>(&ios);
      }
    }

    /// Move constructor
    SerialPod(SerialPod && rhs) : stream_ptr(rhs.stream_ptr), own_ptr(rhs.own_ptr) {
      rhs.own_ptr = false;
    }

    /// Move operator
    SerialPod & operator=(SerialPod && rhs) {
      ClearData();
      stream_ptr  = rhs.stream_ptr;
      own_ptr     = rhs.own_ptr;
      rhs.own_ptr = false;
      return *this;
    }

    // Make sure these are never accidentally created or copied.
    SerialPod()                  = delete;
    SerialPod(const SerialPod &) = delete;

    ~SerialPod() { ClearData(); }

    bool IsLoad() const { return std::holds_alternative<is_ptr_t>(stream_ptr); }

    bool IsSave() const { return std::holds_alternative<os_ptr_t>(stream_ptr); }

    // Load a stand-alone value.
    template <typename T>
    T LoadValue() {
      if constexpr (hasSerializeConstructor<T>) {
        return T(*this);
      } else {
        T temp;
        Load(temp);
        return temp;
      }
    }

    SerialPod & Load() { return *this; }  // Base case...

    SerialPod & Save() { return *this; }  // Base case...

    template <typename T, typename... EXTRA_Ts>
    SerialPod & Load(T & in, EXTRA_Ts &... extras) {
      static_assert(!emp::is_ptr_type<std::decay_t<T>>(),
                    "SerialPod cannot load or save pointers without more information.\n"
                    "Use ManagePtr(value) for restoring pointers by first building the instance,\n"
                    "or LinkPtr(value) to use the value of a pointer that is managed elsewhere.");
      if constexpr (std::is_same<T, std::string>() || std::is_same<T, emp::String>()) {
        std::getline(IStream(), in, '\n');
        in = emp::from_literal_string(in);
      } else if constexpr (hasSerialLoadOverload<T>) {
        SerialLoad(*this, in);
      } else if constexpr (hasSerializeOverload<T>) {
        Serialize(*this, in);
      } else if constexpr (hasSerialLoadMember<T>) {
        in.SerialLoad(*this);
      } else if constexpr (hasSerializeMember<T>) {
        in.Serialize(*this);
      } else if constexpr (std::is_enum<T>()) {  // enums must be converted properly.
        std::string str;
        std::getline(IStream(), str, '\n');
        in = static_cast<T>(std::stoi(str));
      } else if constexpr (canStreamTo<std::ostream, T> && canStreamFrom<std::istream, T>) {
        std::string str;
        std::getline(IStream(), str, '\n');
        std::stringstream ss(str);
        ss >> in;
      } else {
        notify::Error("Invalid SerialPod::Load attempt.");
      };
      return Load(extras...);
    }

    template <typename T, typename... EXTRA_Ts>
    SerialPod & Save(T && in, EXTRA_Ts &&... extras) {
      static_assert(!emp::is_ptr_type<std::decay_t<T>>(),
                    "SerialPod cannot load or save pointers without more information.\n"
                    "Use ManagePtr(value) for restoring pointers by first building the instance,\n"
                    "or LinkPtr(value) to use the value of a pointer that is managed elsewhere.");

      using decayT = std::decay_t<T>;
      if constexpr (std::is_same<decayT, std::string>() || std::is_same<decayT, emp::String>()) {
        OStream() << '\"';
        for (char c : in) { OStream() << emp::to_escaped_string(c); }
        OStream() << "\"\n";
      } else if constexpr (hasSerialSaveOverload<T>) {
        SerialSave(*this, in);
      } else if constexpr (hasSerializeOverload<T>) {
        Serialize(*this, in);
      } else if constexpr (hasSerialSaveMember<T>) {
        in.SerialSave(*this);
      } else if constexpr (hasSerializeMember<T>) {
        in.Serialize(*this);
      } else if constexpr (std::is_enum<decayT>()) {  // enums must be converted to numerical values.
        OStream() << static_cast<int>(in) << '\n';
      } else if constexpr (canStreamTo<std::ostream, T>) {
        OStream() << in << '\n';
      } else {
        notify::Error("Invalid SerialPod::Save attempt on type ", emp::GetTypeName<T>());
      }
      return Save(extras...);
    }

    template <typename T, typename... EXTRA_Ts>
      requires (!emp::AnyConst<T, EXTRA_Ts...>)
    SerialPod & operator()(T && in, EXTRA_Ts &&... extras) {
      if (IsLoad()) {
        // These are run-time errors since it is fine if pod is set to save.
        if constexpr (!emp::has_only_lvalue_reference<T, EXTRA_Ts...>()) {
          emp_assert(false, "Trying to deserialize into other than an lvalue-reference.");
        } else {
          Load(std::forward<T>(in), std::forward<EXTRA_Ts>(extras)...);
        }
      } else {
        Save(std::forward<T>(in), std::forward<EXTRA_Ts>(extras)...);
      }

      return *this;
    }

    // Const overload: save-only.  Any const argument routes here.
    template <typename T, typename... EXTRA_Ts>
    SerialPod & operator()(const T & in, const EXTRA_Ts &... extras) {
      emp_assert(IsSave(), "Trying to deserialize a const value.");
      Save(in, extras...);
      return *this;
    }
  };

  template <typename T>
  SerialPod & SerialLoad(SerialPod & pod, std::vector<T> & vec) {
    const size_t size = pod.LoadValue<size_t>();
    vec.clear();

    if constexpr (hasSerializeConstructor<T>) {
      vec.reserve(size);
      for (size_t i = 0; i < size; ++i) { vec.emplace_back(pod); }
    } else {
      vec.resize(size);
      for (auto & element : vec) { pod.Load(element); }
    }

    return pod;
  }

  template <typename T>
  SerialPod & SerialSave(SerialPod & pod, const std::vector<T> & vec) {
    pod.Save(vec.size());
    for (auto & element : vec) { pod.Save(element); }

    return pod;
  }


  // --- Set-like containers (std::set, std::unordered_set, std::multiset, ...) ---

  template <IsSetContainer C>
  SerialPod & SerialLoad(SerialPod & pod, C & s) {
    using T = typename C::value_type;
    const size_t size = pod.LoadValue<size_t>();
    s.clear();
    for (size_t i = 0; i < size; ++i) { s.insert(pod.LoadValue<T>()); }
    return pod;
  }

  template <IsSetContainer C>
  SerialPod & SerialSave(SerialPod & pod, const C & s) {
    pod.Save(s.size());
    for (const auto & element : s) { pod.Save(element); }
    return pod;
  }


  // --- Map-like containers (std::map, std::unordered_map, std::multimap, ...) ---

  template <IsMapContainer C>
  SerialPod & SerialLoad(SerialPod & pod, C & m) {
    using K = typename C::key_type;
    using V = typename C::mapped_type;
    const size_t size = pod.LoadValue<size_t>();
    m.clear();
    for (size_t i = 0; i < size; ++i) {
      K key = pod.LoadValue<K>();
      V val = pod.LoadValue<V>();
      m.emplace(std::move(key), std::move(val));
    }
    return pod;
  }

  template <IsMapContainer C>
  SerialPod & SerialSave(SerialPod & pod, const C & m) {
    pod.Save(m.size());
    for (const auto & [key, val] : m) { pod.Save(key); pod.Save(val); }
    return pod;
  }


  // --- std::pair ---
  // PAIR_T may be const or non-const; when const, p.first/second are const and route to
  // the save-only const operator() overload automatically.

  template <typename PAIR_T>
    requires requires { typename std::remove_cv_t<PAIR_T>::first_type; }
  SerialPod & Serialize(SerialPod & pod, PAIR_T & p) {
    pod(p.first);
    pod(p.second);
    return pod;
  }


  // --- std::optional ---

  template <typename T>
  SerialPod & SerialLoad(SerialPod & pod, std::optional<T> & opt) {
    const bool has_value = pod.LoadValue<bool>();
    if (has_value) {
      if constexpr (hasSerializeConstructor<T>) {
        opt.emplace(pod);
      } else {
        opt.emplace();
        pod.Load(*opt);
      }
    } else {
      opt.reset();
    }
    return pod;
  }

  template <typename T>
  SerialPod & SerialSave(SerialPod & pod, const std::optional<T> & opt) {
    pod.Save(opt.has_value());
    if (opt.has_value()) { pod.Save(*opt); }
    return pod;
  }


  // --- Fixed-size array containers (std::array, emp::array) ---

  template <IsArrayContainer C>
  SerialPod & SerialLoad(SerialPod & pod, C & arr) {
    const size_t size = pod.LoadValue<size_t>();
    emp_assert(size == arr.size(), size, arr.size());
    for (auto & element : arr) { pod.Load(element); }
    return pod;
  }

  template <IsArrayContainer C>
  SerialPod & SerialSave(SerialPod & pod, const C & arr) {
    pod.Save(arr.size());
    for (const auto & element : arr) { pod.Save(element); }
    return pod;
  }


  // --- std::tuple ---
  // TUPLE_T may be const or non-const; std::apply preserves const, so elements of a
  // const tuple are passed as const refs and route to the save-only operator() overload.

  template <typename TUPLE_T>
    requires (emp::is_template<std::remove_cv_t<TUPLE_T>, std::tuple>::value)
  SerialPod & Serialize(SerialPod & pod, TUPLE_T & t) {
    std::apply([&pod](auto &... elems) { (pod(elems), ...); }, t);
    return pod;
  }


  // --- std::variant ---
  // Index is saved first; load uses an index-sequence fold to reconstruct the right alternative.
  // std::monostate carries no data — only the index is needed to identify it.

  template <typename... Ts>
  SerialPod & SerialLoad(SerialPod & pod, std::variant<Ts...> & v) {
    const size_t index = pod.LoadValue<size_t>();
    emp_assert(index < sizeof...(Ts), index);
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      ([&] {
        if (index == Is) {
          using T = std::variant_alternative_t<Is, std::variant<Ts...>>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            v.template emplace<Is>();
          } else {
            v.template emplace<Is>(pod.LoadValue<T>());
          }
        }
      }(), ...);
    }(std::make_index_sequence<sizeof...(Ts)>{});
    return pod;
  }

  template <typename... Ts>
  SerialPod & SerialSave(SerialPod & pod, const std::variant<Ts...> & v) {
    pod.Save(v.index());
    std::visit([&pod](const auto & val) {
      if constexpr (!std::is_same_v<std::decay_t<decltype(val)>, std::monostate>) {
        pod.Save(val);
      }
    }, v);
    return pod;
  }


}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_SERIALIZE_SERIAL_POD_HPP_GUARD
