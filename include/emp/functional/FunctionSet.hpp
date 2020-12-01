/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file  FunctionSet.hpp
 *  @brief Setup a collection of functions, all with the same signature, that can be run as a group.
 *  @note Status: BETA
 */

#ifndef EMP_FUNCTION_SET_H
#define EMP_FUNCTION_SET_H

#include <functional>
#include "../base/vector.hpp"

namespace emp {

  template <typename T> class FunctionSet;

  /// A vector of functions that can all be triggered at onece; results can either be returned
  /// in a vector or post-processed in a function (such as max, min, etc.)
  /// Derived from emp::vector, hence with all of the same methods as vector.
  template <typename RETURN_T, typename... ARGS>
  class FunctionSet<RETURN_T(ARGS...)> : public emp::vector<std::function<RETURN_T(ARGS...)>> {
  protected:
    mutable emp::vector<RETURN_T> return_vals;

  public:
    FunctionSet() : return_vals() { ; }
    ~FunctionSet() { ; }

    using base_t = emp::vector<std::function<RETURN_T(ARGS...)>>;
    using value_type = typename base_t::value_type;
    using return_t = RETURN_T;

    /// How many functions are in this FunctionSet?
    size_t GetSize() const { return base_t::size(); }

    /// Add a new funtion to this FunctionSet
    void Add(const value_type & in_fun) { base_t::push_back(in_fun); }

    /// Remove the function at a specified position.
    void Remove(size_t pos) { base_t::erase(base_t::begin()+pos); }

    /// Run all functions and return a vector of all results.
    const emp::vector<RETURN_T> & Run(ARGS... args) const {
      const size_t num_tests = base_t::size();
      return_vals.resize(num_tests);
      for (size_t i = 0; i < num_tests; i++) {
        return_vals[i] = (*this)[i](args...);
      }
      return return_vals;
    }

    /// If you want to provide a filter function, you can retrieve a specific return value.
    /// The filter should take in two return values and indicate which is "better".
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

    /// Run all functions and return the highest value.
    RETURN_T FindMax(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return std::max(i1,i2); }, default_val);
    }

    /// Run all functions and return the lowest value.
    RETURN_T FindMin(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return std::min(i1,i2); }, default_val);
    }

    /// Run all functions and return the total value.
    RETURN_T FindSum(ARGS... args, RETURN_T default_val=0) const {
      return Run(args..., [](double i1, double i2){ return i1 + i2; }, default_val);
    }

    /// Run all functions and return a product of all values.
    RETURN_T FindProduct(ARGS... args, RETURN_T default_val=1) const {
      return Run(args..., [](double i1, double i2){ return i1 * i2; }, default_val);
    }
  };


  /// A specialized version of FunctionSet for void functions.
  template <typename... ARGS>
  class FunctionSet<void(ARGS...)> : public emp::vector<std::function<void(ARGS...)>> {
  public:
    FunctionSet() { ; }
    ~FunctionSet() { ; }

    using base_t = emp::vector<std::function<void(ARGS...)>>;
    using value_type = typename base_t::value_type;
    using return_t = void;

    /// How many functions are in this FunctionSet?
    size_t GetSize() const { return base_t::size(); }

    /// Add a new function to this FunctionSet.
    void Add(const std::function<void(ARGS...)> & in_fun) { base_t::push_back(in_fun); }

    /// Remove the function at the designated position from this FunctionSet.
    void Remove(size_t pos) { base_t::erase(base_t::begin()+(int)pos); }

    /// Run all functions in the FunctionSet
    void Run(ARGS... args) const {
      for (const std::function<void(ARGS...)> & cur_fun : *this) {
        cur_fun(args...);
      }
    }
  };

}

#endif
