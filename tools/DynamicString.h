//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A string handler where sections update dynamically based on functions.

#ifndef EMP_DYNAMIC_STRING_H
#define EMP_DYNAMIC_STRING_H

#include <functional>
#include <string>

#include "../base/vector.h"

namespace emp {

  class DynamicString {
  public:
    using value_t = std::function<std::string()>;
  private:
    emp::vector<value_t> fun_set;  // Functions to update strings.

  public:
    DynamicString() { ; }
    DynamicString(const DynamicString &) = default;

    size_t GetSize() const { return fun_set.size(); }
    std::string operator[](size_t id) const { return fun_set[id](); }
    const value_t & GetFunction(size_t id) const { return fun_set[id]; }
    DynamicString & Clear() { fun_set.resize(0); return *this; }

    std::string str() {
      std::stringstream ss;
      for (auto & cur_fun : fun_set) ss << cur_fun();
      return ss.str();
    }

    DynamicString & Set(size_t id, const value_t & in_fun) {
      fun_set[id] = in_fun;
      return *this;
    }

    DynamicString & Set(size_t id, const std::string & in_text) {
      return Set( id, [in_text](){ return in_text; } );
    }

    DynamicString & Append(const value_t & in_fun) {
      fun_set.push_back(in_fun);
      return *this;
    }

    // If a string is appended, automatically create a function that just returns that string.
    DynamicString & Append(const std::string & in_text) {
      return Append( [in_text](){ return in_text; } );
    }

    template <typename IN_TYPE>
    DynamicString & operator<<(IN_TYPE && _in) { return Append(_in); }

  };

}


std::ostream & operator<<( std::ostream & os, const emp::DynamicString & strings )
{
  for (size_t i = 0; i < strings.GetSize(); ++i) {
    os << strings[i];
  }
  return os;
}


#endif
