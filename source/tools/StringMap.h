/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file StringMap.h
 *  @brief An std::map wrapper that deals smootly with strigns and fast compile-time optimizations.
 *  @note Status: ALPHA
 */


#ifndef EMP_STRING_MAP_H
#define EMP_STRING_MAP_H

#include <unordered_map>

#include "string_utils.h"

/// Macro to covert a literal string to a unique ID, mostly at compile time.  Specifically,
/// the string is converted to a unique type at compile time, which is then mapped to a unique
/// function.  That function is run a run-time, but preserves the id to return so it is
/// calculated only once.
#define EMP_STRING_ID(STR)                         \
  ([](){                                           \
    constexpr auto temp = EMP_TEXT_PACK(STR);      \
    return emp::StringID::Get<decltype(temp)>();   \
  }())


namespace emp {

  /// A small class for maintaining unique string IDs.
  class StringID {
  private:
    emp::Ptr<const std::string> str_ptr;   /// Pointer to a unique instance of this string.

    static auto & GetStringSet() { 
      static std::unordered_set< std::string > str_set;
      return str_set;
    }
  public:
    StringID(const StringID &) = default;
    StringID(const std::string & in_string) {
      auto [str_it, success] = GetStringSet().insert(in_string);
      (void) success;           // Prevent unused variable error when not in debug mode.
      str_ptr = &(*str_it);
    }

    size_t ToValue() const { return (size_t) str_ptr; }
    const std::string & ToString() const { return *str_ptr; }

    /// Get a StringID based on a StringType or another type with a 
    /// static ToString() member function.
    template <typename T>
    static StringID Get() {
      static StringID out(T::ToString());
      return out;
    }

    /// Get a StringID based on a string (for completeness; this is the same as the constructor.)
    static StringID Get(const std::string & str) {
      return StringID(str);
    }
  };


  /// A class that wraps maps of strings to allow for effective optimizations.
  
}

#endif
