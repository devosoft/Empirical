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
    /// Get the next unique ID for a type.
    static size_t GetNextTypeValue() {
      static size_t next_type_value = 0;
      return ++next_type_value;
    }
  }

  template <typename T>
  static size_t GetTypeValue() {
    static const size_t type_value = internal::GetNextTypeValue();
    return type_value;
  }

  // Generic TypeID structure for when none of the specialty cases trigger.
  template<typename T> struct TypeID {
    template<typename TEST> using TypeIDFilter = decltype(&TEST::TypeID);
    struct UnknownID { static std::string TypeID() { return "Unknown"; } };
    static std::string GetName() {
      using print_t = typename TypePack<T,UnknownID>::template find_t<TypeIDFilter>;
      return print_t::TypeID();
    }
  };

  // Built-in types.
  template<> struct TypeID<void> { static std::string GetName() { return "void"; } };

  template<> struct TypeID<bool> { static std::string GetName() { return "bool"; } };
  template<> struct TypeID<double> { static std::string GetName() { return "double"; } };
  template<> struct TypeID<float> { static std::string GetName() { return "float"; } };

  template<> struct TypeID<char> { static std::string GetName() { return "char"; } };
  template<> struct TypeID<char16_t> { static std::string GetName() { return "char16_t"; } };
  template<> struct TypeID<char32_t> { static std::string GetName() { return "char32_t"; } };

  template<> struct TypeID<int8_t>  { static std::string GetName() { return "int8_t"; } };
  template<> struct TypeID<int16_t> { static std::string GetName() { return "int16_t"; } };
  template<> struct TypeID<int32_t> { static std::string GetName() { return "int32_t"; } };
  template<> struct TypeID<int64_t> { static std::string GetName() { return "int64_t"; } };
  template<> struct TypeID<uint8_t>  { static std::string GetName() { return "uint8_t"; } };
  template<> struct TypeID<uint16_t> { static std::string GetName() { return "uint16_t"; } };
  template<> struct TypeID<uint32_t> { static std::string GetName() { return "uint32_t"; } };
  template<> struct TypeID<uint64_t> { static std::string GetName() { return "uint64_t"; } };

  // Check for type attributes...
  template<typename T> struct TypeID<T*> {
    static std::string GetName() { return TypeID<T>::GetName() + '*'; }
  };

  // Tools for using TypePack
  template<typename T, typename... Ts> struct TypeID<emp::TypePack<T,Ts...>> {
    static std::string GetTypes() {
      std::string out = TypeID<T>::GetName();
      if (sizeof...(Ts) > 0) out += ",";
      out += TypeID<emp::TypePack<Ts...>>::GetTypes();
      return out;
    }
	  static std::string GetName() {
      std::string out = "emp::TypePack<";
	    out += GetTypes();
	    out += ">";
	    return out;
    }
  };
  template<> struct TypeID< emp::TypePack<> > {
    static std::string GetTypes() { return ""; }
    static std::string GetName() { return "emp::TypePack<>"; }
  };

  // Generic TemplateID structure for when none of the specialty cases trigger.
  template <typename T> struct TemplateID {
    static std::string GetName() { return "UnknownTemplate"; }
  };

  template<template <typename...> class TEMPLATE, typename... Ts>
  struct TypeID<TEMPLATE<Ts...>> {
    static std::string GetName() {
      return TemplateID<TEMPLATE<Ts...>>::GetName()
            + '<' + TypeID<emp::TypePack<Ts...>>::GetTypes() + '>';
    }
  };
}


namespace emp{

  // Standard library types.
  template<> struct TypeID<std::string> { static std::string GetName() { return "std::string"; } };

  // Standard library templates.
  //  template <typename... Ts> struct TemplateID<std::array<Ts...>> { static std::string GetName() { return "array"; } };

  template<typename T, typename... Ts> struct TypeID< emp::vector<T,Ts...> > {
    static std::string GetName() {
      using simple_vt = emp::vector<T>;
      using full_vt = emp::vector<T,Ts...>;
      if (std::is_same<simple_vt,full_vt>::value) {
        return "emp::vector<" + TypeID<T>::GetName() + ">";
      }
      return "emp::vector<" + TypeID<TypePack<T,Ts...>>::GetTypes() + ">";
    }
  };

}

#endif
