//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Setup a collection of functions, all with the same signature, that can be run as a group.

#ifndef EMP_FUNCTION_SET_H
#define EMP_FUNCTION_SET_H

#include <functional>
#include "vector.h"

namespace emp {

  template <typename RETURN_T, typename... ARG_TYPES> class FunctionSet {
  private:
    emp::vector<std::function<RETURN_T(ARG_TYPES...)> > fun_set;
    mutable emp::vector<RETURN_T> return_vals;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    using return_t = RETURN_T;

    int GetSize() const { return (int) fun_set.size(); }
    size_t size() const { return fun_set.size(); }        // For standard lib compatability

    void Add(const std::function<RETURN_T(ARG_TYPES...)> & in_fun) {
      fun_set.push_back(in_fun);
    }

    const emp::vector<RETURN_T> & Run(ARG_TYPES... args) const {
      const size_t num_tests = fun_set.size();
      return_vals.resize(num_tests);
      for (size_t i = 0; i < num_tests; i++) {
        return_vals[i] = (fun_set[i])(args...);
      }
      return return_vals;
    }

    // If you want to provide a filter function, you can retrieve a specific return value.
    // The filter should take in two return values and indicate which is "better".
    RETURN_T Run(ARG_TYPES... params, std::function<RETURN_T(RETURN_T, RETURN_T)> comp_fun,
                    RETURN_T default_val=0) const {
      if (fun_set.size() == 0) return default_val;  // If we have no entries, return the default.

      Run(params...);

      RETURN_T best_found = return_vals[0];
      for (int i = 1; i < return_vals.size(); i++) {
        best_found = comp_fun(best_found, return_vals[i]);
      }

      return best_found;
    }

    RETURN_T FindMax(ARG_TYPES... params, RETURN_T default_val=0) const {
      return Run(params..., [](double i1, double i2){ return std::max(i1,i2); }, default_val);
    }
    RETURN_T FindMin(ARG_TYPES... params, RETURN_T default_val=0) const {
      return Run(params..., [](double i1, double i2){ return std::min(i1,i2); }, default_val);
    }
    RETURN_T FindSum(ARG_TYPES... params, RETURN_T default_val=0) const {
      return Run(params..., [](double i1, double i2){ return i1 + i2; }, default_val);
    }
  };


  // A specialized version for void functions.

  template <typename... ARG_TYPES> class FunctionSet<void, ARG_TYPES...> {
  private:
    emp::vector<std::function<void(ARG_TYPES...)> > fun_set;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    int GetSize() const { return (int) fun_set.size(); }
    size_t size() const { return fun_set.size(); }     // For compatability with STL

    void Add(const std::function<void(ARG_TYPES...)> & in_fun) {
      fun_set.push_back(in_fun);
    }

    void Run(ARG_TYPES... params) const {
      for (const std::function<void(ARG_TYPES...)> & cur_fun : fun_set) {
        cur_fun(params...);
      }
    }
  };

}

#endif
