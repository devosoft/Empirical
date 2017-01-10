//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  A string handler where sections update dynamically based on functions.

#ifndef EMP_DYNAMIC_STRING_SET_H
#define EMP_DYNAMIC_STRING_SET_H

#include <functional>
#include <string>
#include <vector>

namespace emp {

  class DynamicStringSet {
  private:
    std::vector<std::function<std::string()>> fun_set;  // Functions to update strings.

  public:
    DynamicStringSet() { ; }
    DynamicStringSet(const DynamicStringSet &) = default;

    size_t GetSize() const { return fun_set.size(); }
    std::string operator[](size_t id) const { return fun_set[id](); }
    const std::function<std::string()> & GetFunction(size_t id) const { return fun_set[id]; }
    DynamicStringSet & Clear() { fun_set.resize(0); return *this; }

    std::string str() {
      std::stringstream ss;
      for (auto & cur_fun : fun_set) ss << cur_fun();
      return ss.str();
    }

    DynamicStringSet & Set(size_t id, const std::function<std::string()> & in_fun) {
      fun_set[id] = in_fun;
      return *this;
    }

    DynamicStringSet & Set(size_t id, const std::string & in_text) {
      return Set( id, [in_text](){ return in_text; } );
    }

    DynamicStringSet & Append(const std::function<std::string()> & in_fun) {
      fun_set.push_back(in_fun);
      return *this;
    }

    // If a string is appended, automatically create a function that just returns that string.
    DynamicStringSet & Append(const std::string & in_text) {
      return Append( [in_text](){ return in_text; } );
    }

    template <typename IN_TYPE>
    DynamicStringSet & operator<<(IN_TYPE && _in) { return Append(_in); }

  };

}


std::ostream & operator<<( std::ostream & os, const emp::DynamicStringSet & strings )
{
  for (size_t i = 0; i < strings.GetSize(); ++i) {
    os << strings[i];
  }
  return os;
}


#endif
