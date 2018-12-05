/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  StringType.h
 *  @brief A type that maintains compile-time information about a string sequence.
 *
 */


#ifndef EMP_STRING_TYPE_H
#define EMP_STRING_TYPE_H

#include <string>
#include <sstream>

#include "meta.h"
#include "TypeID.h"

/// Convert a literal string to an instance of a StringType
#define EMP_TEXT_PACK(MSG) emp::StringPacketToStringType( [](){ return MSG; } )

/// Setup a type determined by a message.
#define EMP_TEXT_TYPE(TYPE_NAME, MSG)                        \
    auto emp_temp_ ## TYPE_NAME = EMP_TEXT_PACK(MSG);        \
    using TYPE_NAME = decltype(emp_temp_ ## TYPE_NAME)
// I'd prefer for the body of EMP_TEXT_TYPE, but lambdas must be evaluated.
//    decltype(emp::StringPacketToStringType( [](){ return MSG; } ))

/// Convert a literal string to a unique value (counting up from 0 with each string)
#define EMP_TEXT_HASH(MSG)                           \
  [](){                                              \
    constexpr auto temp = EMP_TEXT_PACK(MSG);        \
    return emp::GetTypeValue<decltype(temp)>();      \
  }()


namespace emp {

  // Generic form of StringType (actual types will be made using specializations below)
  template <char... Ts> struct StringType;

  // Template specialization to maintain full information of a non-empty string as a type.
  template <char C1, char... Cs>
  struct StringType<C1,Cs...> {
    static constexpr char FIRST = C1;             ///< Easy access to first character of string.
    constexpr static int SIZE = 1+sizeof...(Cs);  ///< Easy access to string length.

    using this_t = StringType<C1,Cs...>;          ///< The type of the current StringType
    using pop = StringType<Cs...>;                ///< StringType after removing the first char
    template <int C> using push = StringType<C, C1, Cs...>;       ///< Add char to front of string
    template <int C> using push_back = StringType<C1, Cs..., C>;  ///< Add char to back of string

    /// Does StringType contains the char C?
    constexpr static bool Has(char C) { return (C==C1) | pop::Has(C); }

    /// Count the number of occurances of char C in StringType.
    constexpr static int Count(int C) { return pop::Count(C) + (C==C1); }

    /// Determine the position at which C appears in StringType.
    constexpr static int GetID(int C) {
      if (C==C1) return true;
      if (!Has(C)) return false;
      return (1+pop::GetID(C));
    }

    /// Function to retrieve number of elements in StringType
    constexpr static int GetSize() { return SIZE; }

    /// Determine if there are NO chars in an StringType
    constexpr static bool IsEmpty() { return false; }

    /// Determine if all chars in StringType are different from each other.
    constexpr static bool IsUnique() { return pop::IsUnique() && !pop::Has(C1); }

    /// Convert this StringType back to an std::string object (note: NOT constexpr)
    static std::string ToString() {
      std::stringstream ss;
      ss << C1;
      (ss << ... << Cs);
      return ss.str();
    }
  };

  // Empty StringType template specialization
  template <>
  struct StringType<> {
    static constexpr char FIRST = '\0';           ///< Empty string has null as "first" char
    constexpr static int SIZE = 0;                ///< Empty string as no length

    using this_t = StringType<>;                  ///< The type of the current StringType
    // No pop_t; should give error if used on empty string
    template <int C> using push = StringType<C>;       ///< Add char to front of string
    template <int C> using push_back = StringType<C>;  ///< Add char to back of string

    /// Empty StringType does not contain the char C.
    constexpr static bool Has(char C) { return false; }

    /// Empty StringType has 0 occurances of anything.
    constexpr static int Count(int C) { return 0; }

    /// Empty StringType always returns ID -1 for failue to find.
    constexpr static int GetID(int C) { return -1; }

    /// Empty StringType has size 0.
    constexpr static int GetSize() { return 0; }

    /// Empty StringType is, in fact, empty.
    constexpr static bool IsEmpty() { return true; }

    /// Empty StringType is always unique.
    constexpr static bool IsUnique() { return true; }
  };

  constexpr size_t CalcStringSize(const char * in) {
    size_t count = 0;
    while (in[count] != 0) count++;
    return count;
  }

  template <typename T, size_t pos>
  struct StringPacketToStringType_impl {
    static constexpr auto BuildPack(T packet) {
      using recurse_t = decltype( StringPacketToStringType_impl<T,pos-1>::BuildPack(packet) );
      return typename recurse_t::template push_back< (int) (packet()[pos-1]) >();
    }
  };

  template <typename T>
  struct StringPacketToStringType_impl<T,0> {
    static constexpr auto BuildPack(T packet) {
      return StringType<>();
    }
  };

  template <typename T>
  constexpr auto StringPacketToStringType(T packet) {
    return emp::StringPacketToStringType_impl<T,CalcStringSize(packet())>::BuildPack(packet);
  }

}

#endif
