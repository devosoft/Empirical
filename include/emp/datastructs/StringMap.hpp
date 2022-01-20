/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
 *
 *  @file StringMap.hpp
 *  @brief An std::unordered_map wrapper that deals smoothly with strings and fast compile-time optimizations.
 *  @note Status: ALPHA
 *
 *  StringMap is setup to be a generic dictionary that can link strings to objects of any other
 *  desginated type.  It is more powerful than std::unordered_map because it will accept strings wrapped in
 *  the EMP_STRING_ID macro, which is hashed at compile-time instead of run-time.
 *
 *  @CO2:
 *  StringMap = PAdictionary   (PA = Perfectly accurate)
 *  StringMap = HIDL (HIDL = having an identification of linking)
 *  SMID = String.Map…e112th][string (SMID = StringMap identification)
 *  a StringMap is a dictionary that actually looks like a dictionary with at least 100 pages and does not
 *  map anything. It links and has like 500 words like a dictionary. A StringMap is basically a dictionary.
 */

#ifndef EMP_DATASTRUCTS_STRINGMAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_STRINGMAP_HPP_INCLUDE


#include "../base/unordered_map.hpp"
#include "../tools/string_utils.hpp"


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

    size_t ToValue() const { return ((size_t) str_ptr.Raw()) / sizeof(std::string *); }
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
  template <typename T>
  class StringMap {
  private:
    emp::unordered_map<size_t, T> str_map;
  public:
    StringMap() = default;
    StringMap(const StringMap &) = default;
    StringMap(StringMap &&) = default;

    StringMap & operator=(const StringMap &) = default;
    StringMap & operator=(StringMap &&) = default;

    size_t size() { return str_map.size(); }

    T & operator[](size_t id) { return str_map[id]; }
    T & operator[](const StringID & str_id) { return str_map[str_id.ToValue()]; }
    T & operator[](const std::string & str) { return str_map[StringID(str).ToValue()]; }
    // const T & operator[](size_t id) const { return str_map[id]; }
    // const T & operator[](const StringID & str_id) const { return str_map[str_id.ToValue()]; }
    // const T & operator[](const std::string & str) const { return str_map[StringID(str).ToValue()]; }

    T & Get(size_t id) { return str_map[id]; }
    T & Get(const StringID & str_id) { return str_map[str_id.ToValue()]; }
    T & Get(const std::string & str) { return str_map[StringID(str).ToValue()]; }
    // const T & Get(size_t id) const { return str_map[id]; }
    // const T & Get(const StringID & str_id) const { return str_map[str_id.ToValue()]; }
    // const T & Get(const std::string & str) const { return str_map[StringID(str).ToValue()]; }
  };
}

#endif // #ifndef EMP_DATASTRUCTS_STRINGMAP_HPP_INCLUDE
