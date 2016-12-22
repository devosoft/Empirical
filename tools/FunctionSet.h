//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Setup a collection of functions, all with the same signature, that can be run as a group.
//  Derived from emp::vector, hence with all of the same methods as vector.

#ifndef EMP_FUNCTION_SET_H
#define EMP_FUNCTION_SET_H

#include <functional>
#include "vector.h"

namespace emp {

  template <typename RETURN_T, typename... ARGS>
  class FunctionSet : public emp::vector<std::function<RETURN_T(ARGS...)>> {
  private:
    mutable emp::vector<RETURN_T> return_vals;

  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    using base_t = emp::vector<std::function<RETURN_T(ARGS...)>>;
    using value_type = typename base_t::value_type;
    using return_t = RETURN_T;

    size_t GetSize() const { return base_t::size(); }

    void Add(const value_type & in_fun) { base_t::push_back(in_fun); }
    void Remove(size_t pos) { base_t::erase(base_t::begin()+pos); }

    const emp::vector<RETURN_T> & Run(ARGS... args) const {
      const size_t num_tests = base_t::size();
      return_vals.resize(num_tests);
      for (size_t i = 0; i < num_tests; i++) {
        return_vals[i] = (*this)[i](args...);
      }
      return return_vals;
    }

    // If you want to provide a filter function, you can retrieve a specific return value.
    // The filter should take in two return values and indicate which is "better".
    RETURN_T Run(ARGS... args, std::function<RETURN_T(RETURN_T, RETURN_T)> comp_fun,
                    RETURN_T default_val=0) const {
      if (base_t::size() == 0) return default_val;  // If we have no entries, return the default.

      Run(args...);

      RETURN_T best_found = return_vals[0];
      for (size_t i = 1; i < return_vals.size(); i++) {
        best_found = comp_fun(best_found, return_vals[i]);
      }

      return best_found;
    }

    RETURN_T FindMax(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return std::max(i1,i2); }, default_val);
    }
    RETURN_T FindMin(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return std::min(i1,i2); }, default_val);
    }
    RETURN_T FindSum(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return i1 + i2; }, default_val);
    }
  };


  // A specialized version for void functions.

  template <typename... ARGS>
  class FunctionSet<void, ARGS...> : public emp::vector<std::function<void(ARGS...)>> {
  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    using base_t = emp::vector<std::function<void(ARGS...)>>;
    using value_type = typename base_t::value_type;
    using return_t = void;

    size_t GetSize() const { return base_t::size(); }
    void Add(const std::function<void(ARGS...)> & in_fun) { base_t::push_back(in_fun); }
    void Remove(size_t pos) { base_t::erase(base_t::begin()+pos); }

    void Run(ARGS... args) const {
      for (const std::function<void(ARGS...)> & cur_fun : *this) {
        cur_fun(args...);
      }
    }
  };

}

#endif
