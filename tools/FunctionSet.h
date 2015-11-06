// This file is part of Empirical, https://github.com/mercere99/Empirical/, and is 
// Copyright (C) Michigan State University, 2015. It is licensed 
// under the MIT Software license; see doc/LICENSE

#ifndef EMP_FUNCTION_SET_H
#define EMP_FUNCTION_SET_H

#include <functional>
#include <vector>

namespace emp {
  
  template <typename RETURN_TYPE, typename... ARG_TYPES> class FunctionSet {
  private:
    std::vector<std::function<RETURN_TYPE(ARG_TYPES...)> > fun_set;
    std::vector<RETURN_TYPE> return_vals;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    int GetSize() const { return (int) fun_set.size(); }

    void Add(const std::function<RETURN_TYPE(ARG_TYPES...)> & in_fun) {
      fun_set.push_back(in_fun);
    }

    const std::vector<RETURN_TYPE> & Run(ARG_TYPES... params) {
      const int num_tests = (int) fun_set.size();
      return_vals.resize(num_tests);
      for (int i = 0; i < num_tests; i++) {
        return_vals[i] = (fun_set[i])(params...);
      }
      return return_vals;
    }

    // If you want to provide a filter function, you can retrieve a specific return value.
    // The filter should take in two return values and indicate which is "better".
    RETURN_TYPE Run(ARG_TYPES... params, std::function<RETURN_TYPE(RETURN_TYPE, RETURN_TYPE)> comp_fun,
                    RETURN_TYPE default_val=0) {
      if (fun_set.size() == 0) return default_val;  // If we have no entries, return the default.

      Run(params...);

      RETURN_TYPE best_found = return_vals[0];
      for (int i = 1; i < return_vals.size(); i++) {
        best_found = comp_fun(best_found, return_vals[i]);
      }

      return best_found;
    }

    RETURN_TYPE FindMax(ARG_TYPES... params, RETURN_TYPE default_val=0) {
      return Run(params..., [](double i1, double i2){ return std::max(i1,i2); }, default_val);
    }
    RETURN_TYPE FindMin(ARG_TYPES... params, RETURN_TYPE default_val=0) {
      return Run(params..., [](double i1, double i2){ return std::min(i1,i2); }, default_val);
    }
    RETURN_TYPE FindSum(ARG_TYPES... params, RETURN_TYPE default_val=0) {
      return Run(params..., [](double i1, double i2){ return i1 + i2; }, default_val);
    }
  };


  // A specialized version for void functions.

  template <typename... ARG_TYPES> class FunctionSet<void, ARG_TYPES...> {
  private:
    std::vector<std::function<void(ARG_TYPES...)> > fun_set;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    int GetSize() const { return (int) fun_set.size(); }

    void Add(const std::function<void(ARG_TYPES...)> & in_fun) {
      fun_set.push_back(in_fun);
    }

    void Run(ARG_TYPES... params) {
      for (std::function<void(ARG_TYPES...)> & cur_fun : fun_set) {
        cur_fun(params...);
      }
    }
  };

}

#endif
