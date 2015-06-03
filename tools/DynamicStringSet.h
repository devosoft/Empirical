#ifndef EMP_DYNAMIC_STRING_SET_H
#define EMP_DYNAMIC_STRING_SET_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  A string handler that can have sections update dynamically based on functions.
//

#include <functional>
#include <string>
#include <vector>

namespace emp {

  class DynamicStringSet {
  private:
    std::vector<std::string> string_set;
    std::vector<std::function<void()>> fun_set;

  public:
    DynamicStringSet() { ; }
    DynamicStringSet(const DynamicStringSet &) = default;
    DynamicStringSet(const std::string & in_str) { string_set.push_back(in_str); }

    uint32_t GetSize() const { return string_set.size(); }
    uint32_t GetNumFunctions() const { return fun_set.size(); }
    const std::string & operator[](int id) const { return string_set[id]; }
    DynamicStringSet & Clear() { string_set.resize(0); fun_set.resize(0); return *this; }

    DynamicStringSet & Update() { for (auto & cur_fun : fun_set) cur_fun(); }

    DynamicStringSet & Append(const std::string & in_text) {
      string_set.push_back(in_text);
      return *this;
    }

    DynamicStringSet & Append(const std::function<std::string()> & in_fun) {
      int string_id = string_set.size();
      string_set.push_back(in_fun());
      std::string & cur_string = string_set[string_id];
      fun_set.push_back( [&cur_string, in_fun](){ cur_string = in_fun(); }  );
      return *this;
    }
  };

};

#endif
