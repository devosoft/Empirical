#ifndef EMP_DYNAMIC_STRING_SET_H
#define EMP_DYNAMIC_STRING_SET_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  A string handler where sections update dynamically based on functions.
//

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

    int GetSize() const { return (int) fun_set.size(); }
    std::string operator[](int id) const { return fun_set[id](); }
    const std::function<std::string()> & GetFunction(int id) const { return fun_set[id]; }
    DynamicStringSet & Clear() { fun_set.resize(0); return *this; }

    std::string str() {
      std::stringstream ss;
      for (auto & cur_fun : fun_set) ss << cur_fun();
      return ss.str();
    }

    DynamicStringSet & Set(int id, const std::function<std::string()> & in_fun) {
      fun_set[id] = in_fun;
      return *this;
    }

    DynamicStringSet & Set(int id, const std::string & in_text) {
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
  for (int i = 0; i < (int) strings.GetSize(); ++i) {
    os << strings[i];
  }
  return os;
}


#endif
