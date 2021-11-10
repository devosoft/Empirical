/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021
 *
 *  @file TypeID.hpp
 *  @brief TypeID provides an easy way to convert types to strings.
 *
 *  Developer notes:
 *  * Fill out defaults for remaining standard library classes (as possible)
 *  * If a class has a static TypeID_GetName() defined, use that for the name.
 *  * If a type is a template, give access to parameter types.
 *  * If a type is a function, give access to parameter types.
 */

#ifndef EMP_META_TYPEID_HPP_INCLUDE
#define EMP_META_TYPEID_HPP_INCLUDE

#include <cmath>
#include <sstream>
#include <string>

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../tools/string_utils.hpp"

#include "TypePack.hpp"
#include "type_traits.hpp"

namespace emp {

  using namespace std::string_literals;

  // Pre-declare some types and functions.
  struct TypeID;
  template <typename T> static TypeID GetTypeID();

  inline void SetupTypeNames();

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    // Internal class to setup type names on startup.
    struct TypeID_Setup {
      TypeID_Setup() { SetupTypeNames(); }
    };
    static TypeID_Setup _TypeID_Setup;
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Basic TypeID data structure.
  struct TypeID {
    struct Info {
      bool init = false;                     ///< Has this info been initialized yet?
      std::string name = "[unknown type]";   ///< Unique (ideally human-readable) type name

      virtual ~Info() { }

      virtual bool IsAbstract() const { return false; }
      virtual bool IsArithmetic() const { return false; }
      virtual bool IsArray() const { return false; }
      virtual bool IsClass() const { return false; }
      virtual bool IsConst() const { return false; }
      virtual bool IsEmpty() const { return false; }
      virtual bool IsObject() const { return false; }
      virtual bool IsPointer() const { return false; }
      virtual bool IsReference() const { return false; }
      virtual bool IsTrivial() const { return false; }
      virtual bool IsVoid() const { return false; }
      virtual bool IsVolatile() const { return false; }

      virtual bool IsTypePack() const { return false; }

      virtual size_t GetDecayID() const { return 0; }
      virtual size_t GetElementID() const { return 0; }
      virtual size_t GetRemoveConstID() const { return 0; }
      virtual size_t GetRemoveCVID() const { return 0; }
      virtual size_t GetRemoveExtentID() const { return 0; }
      virtual size_t GetRemoveAllExtentsID() const { return 0; }
      virtual size_t GetRemovePtrID() const { return 0; }
      virtual size_t GetRemoveRefID() const { return 0; }
      virtual size_t GetRemoveVolatileID() const { return 0; }

      /// Return the size (in bytes) of objects of this type.
      virtual size_t GetSize() const { return 0; }

      /// Treat the memory referred to by a void pointer as the current type, convert it to a
      /// double, and return that value. (Default to nan if no such conversion is possible.)
      virtual double ToDouble(const emp::Ptr<const void>) const { return std::nan(""); }

      /// Treat the memory referred to by a void pointer as the current type, convert it to a
      /// string, and return that value. (Default to empty if no such conversion is possible.)
      virtual std::string ToString(const emp::Ptr<const void>) const { return ""; }

      /// Take a double and a void pointer, treat the pointer as the correct type, and assign
      /// the double to it (if possible).  Returns success.
      virtual bool FromDouble(double, const emp::Ptr<void>) const { return false; }

      /// Take a string and a void pointer, treat the pointer as the correct type, and assign
      /// the string to it (if possible).  Returns success.
      virtual bool FromString(const std::string &, const emp::Ptr<void>) const { return false; }

      Info() { ; }
      Info(const std::string & in_name) : name(in_name) { ; }
      Info(const Info&) = default;
    };

    template <typename T>
    struct InfoData : public Info {
      bool IsAbstract() const override { return std::is_abstract<T>(); }
      bool IsArithmetic() const override { return std::is_arithmetic<T>(); }
      bool IsArray() const override { return std::is_array<T>(); }
      bool IsClass() const override { return std::is_class<T>(); }
      bool IsConst() const override { return std::is_const<T>(); }
      bool IsEmpty() const override { return std::is_empty<T>(); }
      bool IsObject() const override { return std::is_object<T>(); }
      bool IsPointer() const override { return emp::is_pointer<T>(); } // Not std::is_pointer<T>() to deal with emp::Ptr.
      bool IsReference() const override { return std::is_reference<T>(); }
      bool IsTrivial() const override { return std::is_trivial<T>(); }
      bool IsVoid() const override { return std::is_same<T,void>(); }
      bool IsVolatile() const override { return std::is_volatile<T>(); }

      bool IsTypePack() const override { return emp::is_TypePack<T>(); }

      size_t GetDecayID() const override {
        using decay_t = std::decay_t<T>;
        if constexpr (std::is_same<T, decay_t>()) return (size_t) this;
        else return GetTypeID< decay_t >();
      }
      size_t GetElementID() const override {
        using element_t = emp::element_t<T>;
        if constexpr (std::is_same<T, element_t>()) return (size_t) this;
        else return GetTypeID< element_t >();
      }
      size_t GetRemoveConstID() const override {
        using remove_const_t = std::remove_const_t<T>;
        if constexpr (std::is_same<T, remove_const_t>()) return (size_t) this;
        else return GetTypeID< remove_const_t >();
      }
      size_t GetRemoveCVID() const override {
        using remove_cv_t = std::remove_cv_t<T>;
        if constexpr (std::is_same<T, remove_cv_t>()) return (size_t) this;
        else return GetTypeID< remove_cv_t >();
      }
      size_t GetRemoveExtentID() const override {
        using remove_extent_t = std::remove_extent_t<T>;
        if constexpr (std::is_same<T, remove_extent_t>()) return (size_t) this;
        else return GetTypeID< remove_extent_t >();
      }
      size_t GetRemoveAllExtentsID() const override {
        using remove_all_extents_t = std::remove_all_extents_t<T>;
        if constexpr (std::is_same<T, remove_all_extents_t>()) return (size_t) this;
        else return GetTypeID< remove_all_extents_t >();
      }
      size_t GetRemovePtrID() const override {
        using remove_ptr_t = emp::remove_pointer_t<T>;
        if constexpr (std::is_same<T, remove_ptr_t>()) return (size_t) this;
        else return GetTypeID< remove_ptr_t >();
      }
      size_t GetRemoveRefID() const override {
        using remove_ref_t = std::remove_reference_t<T>;
        if constexpr (std::is_same<T, remove_ref_t>()) return (size_t) this;
        else return GetTypeID< remove_ref_t >();
      }
      size_t GetRemoveVolatileID() const override {
        using remove_volatile_t = std::remove_volatile_t<T>;
        if constexpr (std::is_same<T, remove_volatile_t>()) return (size_t) this;
        else return GetTypeID< remove_volatile_t >();
      }

      size_t GetSize() const override {
        if constexpr (std::is_void<T>()) return 0;
        else return sizeof(T);
      }

      double ToDouble(const emp::Ptr<const void> ptr) const override {
        using base_t = std::decay_t<T>;

        // If this variable has a built-in ToDouble() trait, use it!
        if constexpr (emp::HasToDouble<T>()) {
          return ptr.ReinterpretCast<const base_t>()->ToDouble();
        }

        // If this type is convertable to a double, cast the pointer to the correct type, de-reference it,
        // and then return the conversion.  Otherwise return NaN
        if constexpr (std::is_convertible<T, double>::value) {
          return (double) *ptr.ReinterpretCast<const base_t>();
        }
        else return std::nan("");
      }

      std::string ToString(const emp::Ptr<const void> ptr) const override {
        using base_t = std::decay_t<T>;

        // If this variable has a built-in ToString() trait, use it!
        if constexpr (emp::HasToString<T>()) {
          return ptr.ReinterpretCast<const base_t>()->ToString();
        }

        // If this variable is a string or can be directly converted to a string, do so.
        else if constexpr (std::is_convertible<T, std::string>::value) {
          return (std::string) *ptr.ReinterpretCast<const base_t>();
        }

        // If this variable is a char, treat it as a single-character string.
        else if constexpr (std::is_same<T, char>::value) {
          return std::string(1, (char) *ptr.ReinterpretCast<const base_t>());
        }

        // If this variable is a numeric value, use to_string.
        else if constexpr (std::is_arithmetic<T>::value) {
          return std::to_string( *ptr.ReinterpretCast<const base_t>() );
        }

        else if constexpr (emp::is_emp_vector<T>::value) {
          return emp::ToString( *ptr.ReinterpretCast<const base_t>() );
        }

        // If we made it this far, we don't know how to convert...
        return "[N/A]";
      }

      bool FromDouble(double value, const emp::Ptr<void> ptr) const override {
        using base_t = std::decay_t<T>;

        // If this variable has a built-in FromDouble() trait, use it!
        if constexpr (emp::HasFromDouble<T>()) {
          return ptr.ReinterpretCast<base_t>()->FromDouble(value);
        }

        // If this type is convertable to a double, cast the pointer to the correct type, de-reference it,
        // and then return the conversion.  Otherwise return NaN
        if constexpr (std::is_convertible<double, T>::value) {
          *ptr.ReinterpretCast<base_t>() = (base_t) value;
          return true;
        }

        else return false;
      }

      bool FromString(const std::string & value, const emp::Ptr<void> ptr) const override {
        using base_t = std::decay_t<T>;

        // If this variable has a built-in FromString() trait, use it!
        if constexpr (emp::HasFromString<T>()) {
          return ptr.ReinterpretCast<base_t>()->FromString(value);
        }

        // If this variable is a string or can be directly converted to a string, do so.
        if constexpr (std::is_convertible<std::string, T>::value) {
          *ptr.ReinterpretCast<base_t>() = value;
          return true;
        }

        // If this variable is a char, treat it as a single-character string.
        if constexpr (std::is_same<T, char>::value) {
          if (value.size() > 1) return false;
          *ptr.ReinterpretCast<char>() = value[0];
          return true;
        }

        // If this variable is a numeric value, use from_string.
        else if constexpr (std::is_arithmetic<T>::value) {
          *ptr.ReinterpretCast<base_t>() = (base_t) stod(value);
          return true;
        }

        // If we made it this far, we don't know how to convert...
        return false;
      }
    };


    using info_t = emp::Ptr<TypeID::Info>;
    info_t info_ptr;

    static info_t GetUnknownInfoPtr() { static Info info; return &info; }

    TypeID() : info_ptr(GetUnknownInfoPtr()) { ; }
    TypeID(info_t _info) : info_ptr(_info) { ; }
    TypeID(size_t id) : info_ptr((TypeID::Info *) id) { ; }
    TypeID(const TypeID &) = default;
    ~TypeID() { ; }
    TypeID & operator=(const TypeID &) = default;

    operator size_t() const noexcept { return (info_ptr->init) ? (size_t) info_ptr.Raw() : 0; }
    operator bool() const noexcept { return info_ptr->init; }
    bool operator==(TypeID in) const { return info_ptr == in.info_ptr; }
    bool operator!=(TypeID in) const { return info_ptr != in.info_ptr; }
    bool operator< (TypeID in) const { return info_ptr < in.info_ptr; }
    bool operator<=(TypeID in) const { return info_ptr <=in.info_ptr; }
    bool operator> (TypeID in) const { return info_ptr > in.info_ptr; }
    bool operator>=(TypeID in) const { return info_ptr >= in.info_ptr; }

    /// Get a unique numerical ID for this TypeID object.
    size_t GetID() const { return (size_t) info_ptr.Raw(); }

    /// Get the name generated for this TypeID (ideally unique)
    const std::string & GetName() const { return info_ptr->name; }

    /// Update the name for ALL instances of this TypeID.
    void SetName(const std::string & in_name) { emp_assert(info_ptr); info_ptr->name = in_name; }

    bool IsInitialized() const { return info_ptr->init; }
    void SetInitialized(bool _in=true) { info_ptr->init = _in; }

    bool IsAbstract() const { return info_ptr->IsAbstract(); }
    bool IsArithmetic() const { return info_ptr->IsArithmetic(); }
    bool IsArray() const { return info_ptr->IsArray(); }
    bool IsClass() const { return info_ptr->IsClass(); }
    bool IsConst() const { return info_ptr->IsConst(); }
    bool IsEmpty() const { return info_ptr->IsEmpty(); }
    bool IsObject() const { return info_ptr->IsObject(); }
    bool IsPointer() const { return info_ptr->IsPointer(); }
    bool IsReference() const { return info_ptr->IsReference(); }
    bool IsTrivial() const { return info_ptr->IsTrivial(); }
    bool IsVoid() const { return info_ptr->IsVoid(); }
    bool IsVolatile() const { return info_ptr->IsVolatile(); }

    bool IsTypePack() const { return info_ptr->IsTypePack(); }

    template <typename T> bool IsType() const { return *this == GetTypeID<T>(); }

    template <typename T, typename... Ts>
    bool IsTypeIn() const {
      if (IsType<T>()) return true;
      if constexpr (sizeof...(Ts) > 0) return IsTypeIn<Ts...>();
      else return false;
    }

    TypeID GetDecayTypeID() const { return info_ptr->GetDecayID(); }
    TypeID GetElementTypeID() const { return info_ptr->GetElementID(); }
    TypeID GetRemoveConstTypeID() const { return info_ptr->GetRemoveConstID(); }
    TypeID GetRemoveCVTypeID() const { return info_ptr->GetRemoveCVID(); }
    TypeID GetRemoveExtentTypeID() const { return info_ptr->GetRemoveExtentID(); }
    TypeID GetRemoveAllExtentsTypeID() const { return info_ptr->GetRemoveAllExtentsID(); }
    TypeID GetRemovePointerTypeID() const { return info_ptr->GetRemovePtrID(); }
    TypeID GetRemoveReferenceTypeID() const { return info_ptr->GetRemoveRefID(); }
    TypeID GetRemoveVolatileTypeID() const { return info_ptr->GetRemoveVolatileID(); }

    size_t GetSize() const { return info_ptr->GetSize(); }

    double ToDouble(const emp::Ptr<const void> ptr) const { return info_ptr->ToDouble(ptr); }
    std::string ToString(const emp::Ptr<const void> ptr) const { return info_ptr->ToString(ptr); }
    bool FromDouble(double value, const emp::Ptr<void> ptr) {
      return info_ptr->FromDouble(value, ptr);
    }
    bool FromString(const std::string & value, const emp::Ptr<void> ptr) {
      return info_ptr->FromString(value, ptr);
    }
  };

  template <typename T> static emp::Ptr<TypeID::Info> BuildInfo();

  /// Retrieve the correct TypeID for a given type.
  template <typename T>
  static TypeID GetTypeID() {
    static emp::Ptr<TypeID::Info> info = BuildInfo<T>();  // Create static info so that it is persistent.
    return TypeID(info);
  }

  /// Retrieve a vector of TypeIDs for a pack of types passed in.
  template <typename... Ts>
  emp::vector<TypeID> GetTypeIDs() {
    return emp::vector<TypeID>{GetTypeID<Ts>()...};
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    // Unimplemented base class -- TypePackIDs are meaningless for non-TypePack classes.
    template <typename T> struct TypePackIDs_impl { };

    // Decompose a TypePack and provide a vector of the individual TypeIDs.
    template <typename... Ts>
    struct TypePackIDs_impl<TypePack<Ts...>> {
      static emp::vector<TypeID> GetIDs() { return GetTypeIDs<Ts...>(); }
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Retrieve a vector of TypeIDs for a TypePack of types passed in.
  template <typename T>
  emp::vector<TypeID> GetTypePackIDs() {
    return internal::TypePackIDs_impl<T>::GetIDs();
  }

  /// Build the information for a single TypeID.
  template <typename T>
  static emp::Ptr<TypeID::Info> BuildInfo() {
    static TypeID::InfoData<T> info;
    if (info.init == false) {
      TypeID type_id(&info);

      info.init = true;
      info.name = typeid(T).name();

      // Now, fix the name if we can be more precise about it.
      if constexpr (std::is_const<T>()) {
        info.name = "const "s + type_id.GetRemoveConstTypeID().GetName();
      }
      else if constexpr (std::is_volatile<T>()) {
        info.name = "volatile "s + type_id.GetRemoveVolatileTypeID().GetName();
      }
      else if constexpr (std::is_array<T>()) {
        info.name = type_id.GetRemoveAllExtentsTypeID().GetName();
        if constexpr (std::rank<T>::value > 0) info.name += "[" + emp::to_string(std::extent<T,0>::value) + "]";
        if constexpr (std::rank<T>::value > 1) info.name += "[" + emp::to_string(std::extent<T,1>::value) + "]";
        if constexpr (std::rank<T>::value > 2) info.name += "[" + emp::to_string(std::extent<T,2>::value) + "]";
        if constexpr (std::rank<T>::value > 3) info.name += "[" + emp::to_string(std::extent<T,3>::value) + "]";
        if constexpr (std::rank<T>::value > 4) info.name += "[" + emp::to_string(std::extent<T,4>::value) + "]";
        if constexpr (std::rank<T>::value > 5) info.name += "[...]";
      }
      else if constexpr (emp::is_pointer<T>()) {
        info.name = type_id.GetRemovePointerTypeID().GetName() + '*';
      }
      else if constexpr (std::is_reference<T>()) {
        info.name = type_id.GetRemoveReferenceTypeID().GetName() + '&';
      }
      else if constexpr (emp::is_emp_vector<T>()) {
        info.name = "vector<"s + type_id.GetElementTypeID().GetName() + '>';
      }
      else if constexpr (emp::is_TypePack<T>()) {
        emp::vector<TypeID> ids = GetTypePackIDs<T>();
        info.name = "TypePack<";
        for (size_t i = 0; i < ids.size(); i++) {
          if (i) info.name += ",";
          info.name += ids[i].GetName();
        }
        info.name += ">";
      }
    }

    return &info;
  }

  /// Setup a bunch of standard type names to be more readable.
  void SetupTypeNames() {

    // Built-in types.
    GetTypeID<void>().SetName("void");

    GetTypeID<bool>().SetName("bool");
    GetTypeID<double>().SetName("double");
    GetTypeID<float>().SetName("float");

    GetTypeID<char>().SetName("char");
    GetTypeID<char16_t>().SetName("char16_t");
    GetTypeID<char32_t>().SetName("char32_t");

    GetTypeID<int8_t>().SetName("int8_t");
    GetTypeID<int16_t>().SetName("int16_t");
    GetTypeID<int32_t>().SetName("int32_t");
    GetTypeID<int64_t>().SetName("int64_t");

    GetTypeID<uint8_t>().SetName( "uint8_t");
    GetTypeID<uint16_t>().SetName("uint16_t");
    GetTypeID<uint32_t>().SetName("uint32_t");
    GetTypeID<uint64_t>().SetName("uint64_t");

    // Standard library types.
    GetTypeID<std::string>().SetName("std::string");

  }
}


namespace std {
  /// Hash function to allow TypeID to be used with maps and sets (must be in std).
  template <>
  struct hash<emp::TypeID> {
    std::size_t operator()(const emp::TypeID & id) const {
      return id.GetID();
    }
  };

  /// operator<< to work with ostream (must be in std to work)
  inline std::ostream & operator<<(std::ostream & out, const emp::TypeID & id) {
    out << id.GetName();
    return out;
  }
}

#endif // #ifndef EMP_META_TYPEID_HPP_INCLUDE
