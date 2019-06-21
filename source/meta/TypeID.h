//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeID provides an easy way to convert types to strings.
//
//
//  Developer notes:
//  * Fill out remaining standard library classes (as possible)
//  * Default to type_traits typeid rather than Unknown

#ifndef EMP_TYPE_ID_H
#define EMP_TYPE_ID_H

#include <sstream>
#include <string>

#include "../base/vector.h"
#include "TypePack.h"


namespace emp {

  namespace internal {
    struct TypeID_Info {
      std::string name;
      TypeID_Info(const std::string & in_name) : name(in_name) { ; }
    };

    static emp::vector<TypeID_Info> & GetTypeID_Info_vector() {
      static emp::vector<TypeID_Info> info_vec({{"Unknown Type"}});
      return info_vec;
    }

    /// Get the next unique ID for a type.
    static size_t GetNextTypeValue(const std::string & name) {
      GetTypeID_Info_vector().push_back(name);
      return GetTypeID_Info_vector().size() - 1;
    }
  }

  struct TypeID {
    size_t id;

    TypeID(size_t _id) : id(_id) { ; }
    TypeID(const TypeID &) = default;
    ~TypeID() { ; }
    TypeID & operator=(const TypeID &) = default;

    operator size_t() const noexcept { return id; }
    bool operator==(TypeID in) const { return id == in.id; }
    bool operator!=(TypeID in) const { return id != in.id; }

    const std::string & GetName() const { return internal::GetTypeID_Info_vector()[id].name; }
    void SetName(const std::string & in_name) { internal::GetTypeID_Info_vector()[id].name = in_name; }
  };

  template <typename T>
  static TypeID GetTypeID() {
    static const size_t type_value = internal::GetNextTypeValue(typeid(T).name());
    return type_value;
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

    // @CAO -- we can actually establish these links when building types...
    // // Check for type attributes...
    // template<typename T> struct TypeID<T*> {
    //   static std::string GetName() { return TypeID<T>::GetName() + '*'; }
    // };

    // // Tools for using TypePack
    // template<typename T, typename... Ts> struct TypeID<emp::TypePack<T,Ts...>> {
    //   static std::string GetTypes() {
    //     std::string out = TypeID<T>::GetName();
    //     if (sizeof...(Ts) > 0) out += ",";
    //     out += TypeID<emp::TypePack<Ts...>>::GetTypes();
    //     return out;
    //   }
    //   static std::string GetName() {
    //     std::string out = "emp::TypePack<";
    //     out += GetTypes();
    //     out += ">";
    //     return out;
    //   }
    // };
    // template<> struct TypeID< emp::TypePack<> > {
    //   static std::string GetTypes() { return ""; }
    //   static std::string GetName() { return "emp::TypePack<>"; }
    // };

    // // Generic TemplateID structure for when none of the specialty cases trigger.
    // template <typename T> struct TemplateID {
    //   static std::string GetName() { return "UnknownTemplate"; }
    // };

    // template<template <typename...> class TEMPLATE, typename... Ts>
    // struct TypeID<TEMPLATE<Ts...>> {
    //   static std::string GetName() {
    //     return TemplateID<TEMPLATE<Ts...>>::GetName()
    //           + '<' + TypeID<emp::TypePack<Ts...>>::GetTypes() + '>';
    //   }
    // };
  }
}

// namespace emp{


//   // Standard library templates.
//   //  template <typename... Ts> struct TemplateID<std::array<Ts...>> { static std::string GetName() { return "array"; } };

//   template<typename T, typename... Ts> struct TypeID< emp::vector<T,Ts...> > {
//     static std::string GetName() {
//       using simple_vt = emp::vector<T>;
//       using full_vt = emp::vector<T,Ts...>;
//       if (std::is_same<simple_vt,full_vt>::value) {
//         return "emp::vector<" + TypeID<T>::GetName() + ">";
//       }
//       return "emp::vector<" + TypeID<TypePack<T,Ts...>>::GetTypes() + ">";
//     }
//   };

//}

#endif
