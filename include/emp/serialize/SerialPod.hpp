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

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <locale>
#include <concepts>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <utility>

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
  public:
    enum class Format { TEXT, BINARY };

  private:
    Format format = Format::TEXT;
    size_t max_container_bytes = 64 * 1024 * 1024;
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

    void SaveUnsigned(uint64_t value) {
      do {
        const unsigned char byte = (value & 0x7f) | (value > 0x7f ? 0x80 : 0);
        OStream().put(static_cast<char>(byte));
        value >>= 7;
      } while (value);
    }

    uint64_t LoadUnsigned() {
      uint64_t value = 0;
      for (unsigned shift = 0; shift < 64; shift += 7) {
        const int next = IStream().get();
        if (next == std::char_traits<char>::eof()) { Fail(); return 0; }
        const auto byte = static_cast<unsigned char>(next);
        if (shift == 63 && byte > 1) { Fail(); return 0; }
        value |= uint64_t(byte & 0x7f) << shift;
        if (!(byte & 0x80)) return value;
      }
      Fail();
      return 0;
    }

    template <typename T> void SaveScalar(T value) {
      if constexpr (std::is_enum_v<T>) {
        SaveScalar(static_cast<std::underlying_type_t<T>>(value));
      } else if constexpr (std::integral<T>) {
        if constexpr (sizeof(T) == 1) OStream().put(static_cast<char>(value));
        else if constexpr (std::signed_integral<T>) {
          SaveUnsigned(value < 0 ? uint64_t(-(value + 1)) * 2 + 1 : uint64_t(value) * 2);
        } else SaveUnsigned(value);
      } else {
        if constexpr ((sizeof(T) == 4 || sizeof(T) == 8) && std::numeric_limits<T>::is_iec559) {
          auto bytes = std::bit_cast<std::array<unsigned char, sizeof(T)>>(value);
          if constexpr (std::endian::native == std::endian::big) std::reverse(bytes.begin(), bytes.end());
          OStream().write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
        } else Fail(); // Other floating-point types remain available in text mode.
      }
    }

    template <typename T> void LoadScalar(T & value) {
      if constexpr (std::is_enum_v<T>) {
        std::underlying_type_t<T> raw{};
        LoadScalar(raw);
        if (Good()) value = static_cast<T>(raw);
      } else if constexpr (std::integral<T>) {
        if constexpr (sizeof(T) == 1) {
          const int byte = IStream().get();
          if (byte == std::char_traits<char>::eof()
              || (std::same_as<T, bool> && byte > 1)) { Fail(); return; }
          value = static_cast<T>(byte);
        } else {
          const uint64_t raw = LoadUnsigned();
          if (!Good()) return;
          if constexpr (std::signed_integral<T>) {
            const int64_t decoded = (raw & 1) ? -int64_t(raw >> 1) - 1 : int64_t(raw >> 1);
            if (decoded < std::numeric_limits<T>::min() || decoded > std::numeric_limits<T>::max()) {
              Fail(); return;
            }
            value = static_cast<T>(decoded);
          } else {
            if (raw > std::numeric_limits<T>::max()) { Fail(); return; }
            value = static_cast<T>(raw);
          }
        }
      } else {
        if constexpr ((sizeof(T) == 4 || sizeof(T) == 8) && std::numeric_limits<T>::is_iec559) {
          std::array<unsigned char, sizeof(T)> bytes{};
          IStream().read(reinterpret_cast<char *>(bytes.data()), bytes.size());
          if (!Good()) return;
          if constexpr (std::endian::native == std::endian::big) std::reverse(bytes.begin(), bytes.end());
          value = std::bit_cast<T>(bytes);
        } else Fail();
      }
    }

  public:
    SerialPod(std::ostream & os, Format encoding = Format::TEXT)
      : format(encoding), stream_ptr(emp::ToPtr(&os)) { }

    SerialPod(std::istream & is, Format encoding = Format::TEXT)
      : format(encoding), stream_ptr(emp::ToPtr(&is)) { }

    SerialPod(std::iostream & ios, bool is_save, Format encoding = Format::TEXT) : format(encoding) {
      if (is_save) {
        stream_ptr = emp::ToPtr<std::ostream>(&ios);
      } else {
        stream_ptr = emp::ToPtr<std::istream>(&ios);
      }
    }

    /// Move constructor
    SerialPod(SerialPod && rhs) : format(rhs.format), max_container_bytes(rhs.max_container_bytes)
      , stream_ptr(rhs.stream_ptr), own_ptr(rhs.own_ptr) {
      rhs.own_ptr = false;
    }

    /// Move operator
    SerialPod & operator=(SerialPod && rhs) {
      ClearData();
      format = rhs.format;
      max_container_bytes = rhs.max_container_bytes;
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

    [[nodiscard]] bool IsBinary() const { return format == Format::BINARY; }
    [[nodiscard]] bool Good() const {
      return std::visit([](auto ptr) { return ptr && bool(*ptr); }, stream_ptr);
    }
    void Fail() {
      std::visit([](auto ptr) { ptr->setstate(std::ios::failbit); }, stream_ptr);
    }
    void SetMaxContainerBytes(size_t bytes) { max_container_bytes = bytes; }
    bool CheckContainerSize(uint64_t count, size_t element_bytes = 1) {
      if (!Good() || !element_bytes || count > max_container_bytes / element_bytes) {
        Fail(); return false;
      }
      return true;
    }

    // Load a stand-alone value.
    template <typename T>
    T LoadValue() {
      if constexpr (hasSerializeConstructor<T>) {
        return T(*this);
      } else {
        T temp{};
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
      if (!Good()) return *this;
      if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, emp::String>) {
        if (IsBinary()) {
          const uint64_t count = LoadUnsigned();
          if (!CheckContainerSize(count)) return *this;
          in.resize(static_cast<size_t>(count));
          IStream().read(in.data(), static_cast<std::streamsize>(count));
        } else {
          std::string literal;
          if (!std::getline(IStream(), literal) || literal.size() < 2
              || literal.front() != '"' || literal.back() != '"') { Fail(); return *this; }
          in = emp::from_literal_string(literal);
        }
      } else if constexpr (hasSerialLoadOverload<T>) {
        SerialLoad(*this, in);
      } else if constexpr (hasSerializeOverload<T>) {
        Serialize(*this, in);
      } else if constexpr (hasSerialLoadMember<T>) {
        in.SerialLoad(*this);
      } else if constexpr (hasSerializeMember<T>) {
        in.Serialize(*this);
      } else if constexpr (std::is_enum_v<T>) {
        if (IsBinary()) LoadScalar(in);
        else {
          auto raw = LoadValue<std::underlying_type_t<T>>();
          if (Good()) in = static_cast<T>(raw);
        }
      } else if constexpr (canStreamTo<std::ostream, T> && canStreamFrom<std::istream, T>) {
        if constexpr (std::is_arithmetic_v<T>) {
          if (IsBinary()) { LoadScalar(in); return Load(extras...); }
        }
        std::string line;
        if (!std::getline(IStream(), line)) { Fail(); return *this; }
        std::istringstream input{line};
        input.imbue(std::locale::classic());
        T decoded{};
        if constexpr (std::floating_point<T>) {
          // Floating-point from_chars requires newer macOS runtimes than Avida supports.
          // Streams need explicit nonfinite tokens, and some libc++ versions mark a
          // correctly rounded subnormal as a parse failure.
          if (line == "inf" || line == "infinity") decoded = std::numeric_limits<T>::infinity();
          else if (line == "-inf" || line == "-infinity") decoded = -std::numeric_limits<T>::infinity();
          else if (line == "nan" || line == "-nan") decoded = std::numeric_limits<T>::quiet_NaN();
          else {
            input >> std::noskipws >> decoded;
            const bool subnormal = decoded != 0 && std::isfinite(decoded)
                                   && std::abs(decoded) < std::numeric_limits<T>::min();
            if ((!input && !subnormal) || !input.eof()) { Fail(); return *this; }
          }
        } else {
          input >> std::ws;
          if constexpr (std::unsigned_integral<T>) {
            if (input.peek() == '-') { Fail(); return *this; }
          }
          if (!(input >> decoded)) { Fail(); return *this; }
          input >> std::ws;
          if (!input.eof()) { Fail(); return *this; }
        }
        in = std::move(decoded);
      } else {
        static_assert(emp::dependent_false<T>::value, "SerialPod cannot load this type.");
      }
      return Load(extras...);
    }

    template <typename T, typename... EXTRA_Ts>
    SerialPod & Save(T && in, EXTRA_Ts &&... extras) {
      static_assert(!emp::is_ptr_type<std::decay_t<T>>(),
                    "SerialPod cannot load or save pointers without more information.\n"
                    "Use ManagePtr(value) for restoring pointers by first building the instance,\n"
                    "or LinkPtr(value) to use the value of a pointer that is managed elsewhere.");

      if (!Good()) return *this;
      using decayT = std::decay_t<T>;
      if constexpr (std::is_same<decayT, std::string>() || std::is_same<decayT, emp::String>()) {
        if (IsBinary()) {
          SaveUnsigned(in.size());
          OStream().write(in.data(), static_cast<std::streamsize>(in.size()));
          return Save(extras...);
        }
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
        if (IsBinary()) SaveScalar(in);
        else Save(static_cast<std::underlying_type_t<decayT>>(in));
      } else if constexpr (canStreamTo<std::ostream, T>) {
        if constexpr (std::is_arithmetic_v<decayT>) {
          if (IsBinary()) { SaveScalar(in); return Save(extras...); }
        }
        if constexpr (std::floating_point<decayT>) {
          const auto precision = OStream().precision();
          OStream() << std::setprecision(std::numeric_limits<decayT>::max_digits10) << in << '\n';
          OStream().precision(precision);
        } else OStream() << in << '\n';
      } else {
        static_assert(emp::dependent_false<T>::value,
          "SerialPod cannot save this type: no Serialize/SerialSave member or overload found,\n"
          "and type is not an enum or streamable via operator<<.");
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

    /// Const overload: save-only.  Any const argument routes here.
    template <typename T, typename... EXTRA_Ts>
    SerialPod & operator()(const T & in, const EXTRA_Ts &... extras) {
      emp_assert(IsSave(), "Trying to deserialize a const value.");
      Save(in, extras...);
      return *this;
    }

    /// Serialize a value as a type that may be different from itself.
    /// For example, an int8_t would save as a char, but you may want to save it as an int.
    template <typename SAVE_T, typename VAR_T>
      requires std::convertible_to<VAR_T, SAVE_T> && std::convertible_to<SAVE_T, VAR_T>
    SerialPod & SerializeAs(VAR_T & in) {
      return UseAccessors([&in]{ return static_cast<SAVE_T>(in); },
                          [this, &in](SAVE_T v){
                            if constexpr (std::integral<SAVE_T> && std::integral<VAR_T>) {
                              if (!std::in_range<VAR_T>(v)) { Fail(); return; }
                            }
                            in = static_cast<VAR_T>(v);
                          });
    }
    
    /// Serialize through an accessor pair rather than a directly-referenced variable.
    /// On save, `get_fun()` supplies the value to write; on load, a value is read back and
    /// handed to `set_fun(value)`.  The serialized type is the decayed return type of
    /// `get_fun`, so `set_fun` must accept a value of that type.  This lets a class expose a
    /// computed or indirectly-stored quantity for serialization without a backing variable.
    ///
    /// Example:
    ///   pod.UseAccessors([&]{ return v.size(); }, [&](size_t n){ v.resize(n); });
    template <typename GET_FUN, typename SET_FUN>
      requires std::invocable<GET_FUN> &&
               (!std::is_void_v<std::invoke_result_t<GET_FUN>>) &&
               std::invocable<SET_FUN, std::invoke_result_t<GET_FUN>>
    SerialPod & UseAccessors(GET_FUN && get_fun, SET_FUN && set_fun) {
      using value_t = std::decay_t<std::invoke_result_t<GET_FUN>>;
      if (IsSave()) {
        Save(std::invoke(std::forward<GET_FUN>(get_fun)));
      } else {
        auto value = LoadValue<value_t>();
        if (Good()) std::invoke(std::forward<SET_FUN>(set_fun), std::move(value));
      }
      return *this;
    }

    /// A specialized version of UseAccessors for containers to maintain container size.
    /// On save, writes `in.size()`; on load, calls `in.resize()` to restore the element count.
    /// Constrained to types that expose a size()-and-resize() interface (e.g. std::vector,
    /// std::string, emp::vector) so misuse fails cleanly at the call site.
    SerialPod & UseSizeAccessors(auto & in)
      requires requires(size_t s) {
        { in.size() } -> std::convertible_to<size_t>;
        in.resize(s);
      }
    {
      return UseAccessors([&in]{ return in.size(); }, [this, &in](size_t s){
        if (CheckContainerSize(s, sizeof(typename std::remove_reference_t<decltype(in)>::value_type)))
          in.resize(s);
      });
    }
  };

  template <typename T>
  SerialPod & SerialLoad(SerialPod & pod, std::vector<T> & vec) {
    const size_t size = pod.LoadValue<size_t>();
    if (!pod.CheckContainerSize(size, sizeof(T))) return pod;
    vec.clear();

    if constexpr (hasSerializeConstructor<T>) {
      vec.reserve(size);
      for (size_t i = 0; i < size && pod.Good(); ++i) { vec.emplace_back(pod); }
    } else {
      vec.resize(size);
      for (auto & element : vec) { pod.Load(element); }
    }

    return pod;
  }

  template <typename T>
  SerialPod & SerialSave(SerialPod & pod, std::vector<T> & vec) {
    pod.Save(vec.size());
    for (auto & element : vec) { pod.Save(element); }

    return pod;
  }


  // --- Set-like containers (std::set, std::unordered_set, std::multiset, ...) ---

  template <IsSetContainer SET_T>
  SerialPod & SerialLoad(SerialPod & pod, SET_T & set) {
    using value_t = typename SET_T::value_type;
    const size_t size = pod.LoadValue<size_t>();
    if (!pod.CheckContainerSize(size, sizeof(value_t))) return pod;
    set.clear();
    for (size_t i = 0; i < size && pod.Good(); ++i) { set.insert(pod.LoadValue<value_t>()); }
    return pod;
  }

  template <IsSetContainer SET_T>
  SerialPod & SerialSave(SerialPod & pod, SET_T & set) {
    pod.Save(set.size());
    for (auto & element : set) { pod.Save(element); }
    return pod;
  }


  // --- Map-like containers (std::map, std::unordered_map, std::multimap, ...) ---

  template <IsMapContainer MAP_T>
  SerialPod & SerialLoad(SerialPod & pod, MAP_T & m) {
    using K = typename MAP_T::key_type;
    using V = typename MAP_T::mapped_type;
    const size_t size = pod.LoadValue<size_t>();
    if (!pod.CheckContainerSize(size, sizeof(typename MAP_T::value_type))) return pod;
    m.clear();
    for (size_t i = 0; i < size && pod.Good(); ++i) {
      K key = pod.LoadValue<K>();
      V val = pod.LoadValue<V>();
      m.emplace(std::move(key), std::move(val));
    }
    return pod;
  }

  template <IsMapContainer MAP_T>
  SerialPod & SerialSave(SerialPod & pod, MAP_T & m) {
    pod.Save(m.size());
    for (auto & [key, val] : m) { pod.Save(key); pod.Save(val); }
    return pod;
  }


  // --- std::pair ---

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
  SerialPod & SerialSave(SerialPod & pod, std::optional<T> & opt) {
    pod.Save(opt.has_value());
    if (opt.has_value()) { pod.Save(*opt); }
    return pod;
  }


  // --- Fixed-size array containers (std::array, emp::array) ---

  template <IsArrayContainer ARRAY_T>
  SerialPod & SerialLoad(SerialPod & pod, ARRAY_T & arr) {
    [[maybe_unused]] const size_t size = pod.LoadValue<size_t>();
    if (!pod.Good() || size != arr.size()) { pod.Fail(); return pod; }
    for (auto & element : arr) { pod.Load(element); }
    return pod;
  }

  template <IsArrayContainer ARRAY_T>
  SerialPod & SerialSave(SerialPod & pod, ARRAY_T & arr) {
    pod.Save(arr.size());
    for (auto & element : arr) { pod.Save(element); }
    return pod;
  }


  // --- std::tuple ---

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
    if (!pod.Good() || index >= sizeof...(Ts)) { pod.Fail(); return pod; }
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
  SerialPod & SerialSave(SerialPod & pod, std::variant<Ts...> & v) {
    pod.Save(v.index());
    std::visit([&pod](auto & val) {
      if constexpr (!std::is_same_v<std::decay_t<decltype(val)>, std::monostate>) {
        pod.Save(val);
      }
    }, v);
    return pod;
  }


}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_SERIALIZE_SERIAL_POD_HPP_GUARD
