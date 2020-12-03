/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  DynamicString.hpp
 *  @brief A string handler where sections update dynamically based on functions.
 *  @note Status: BETA
 */


#ifndef EMP_DYNAMIC_STRING_H
#define EMP_DYNAMIC_STRING_H

#include <functional>
#include <string>

#include "../base/vector.hpp"

namespace emp {

  /// A string handler where some sections can be fixed strings, while others update
  /// dynamically based on functions.
  class DynamicString {
  public:
    using value_t = std::function<std::string()>;
  private:
    emp::vector<value_t> fun_set;  // Functions to update strings.

  public:
    DynamicString() { ; }
    DynamicString(const DynamicString &) = default;

    /// How many string components (funcations or continuous substrings) are in this DynamicString?
    size_t GetSize() const { return fun_set.size(); }

    /// Index in to a specific component (not a specific character, since size is variable)
    /// and return it's associated string.
    std::string operator[](size_t id) const { return fun_set[id](); }

    /// Index in to a specific component (not a specific character, since size is variable)
    /// and return it's associated function.
    const value_t & GetFunction(size_t id) const { return fun_set[id]; }

    /// Remove all contents on this DynamicString
    DynamicString & Clear() { fun_set.clear(); return *this; }

    /// Convert to an std::string.
    std::string str() {
      std::stringstream ss;
      for (auto & cur_fun : fun_set) ss << cur_fun();
      return ss.str();
    }

    /// Set the value of a specified component to the provided function.
    DynamicString & Set(size_t id, const value_t & in_fun) {
      fun_set[id] = in_fun;
      return *this;
    }

    /// Set the value of a specified component to the provided std::string text.
    DynamicString & Set(size_t id, const std::string & in_text) {
      return Set( id, [in_text](){ return in_text; } );
    }

    /// Add a new function to the end of the DynamicString.
    DynamicString & Append(const value_t & in_fun) {
      fun_set.push_back(in_fun);
      return *this;
    }

    /// Add new std::string text to the end of the DynamicString.
    // (automatically create a function that just returns that string.)
    DynamicString & Append(const std::string & in_text) {
      return Append( [in_text](){ return in_text; } );
    }

    /// Allow operator<< to append to the back of a DynamicString.
    template <typename IN_TYPE>
    DynamicString & operator<<(IN_TYPE && _in) { return Append(_in); }

  };

}

namespace std {
  /// Make sure that DynamicString works with with std::ostream.
  std::ostream & operator<<( std::ostream & os, const emp::DynamicString & strings )
  {
    for (size_t i = 0; i < strings.GetSize(); ++i) {
      os << strings[i];
    }
    return os;
  }
}

#endif
