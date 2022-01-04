/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019
 *
 *  @file stats.hpp
 *  @brief Functions for calculating various statistics about an ensemble.
 *  @note Status: BETA
 */

#ifndef EMP_MATH_STATS_HPP_INCLUDE
#define EMP_MATH_STATS_HPP_INCLUDE

#include <cmath>
#include <functional>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../base/map.hpp"
#include "../base/vector.hpp"
#include "../datastructs/vector_utils.hpp"
#include "../meta/type_traits.hpp"

#include "math.hpp"

namespace emp {

  /// Calculate sum of the values in a container; if pointers to scalars, convert to scalar type
  template <typename C>
  typename emp::remove_ptr_type<typename C::value_type>::type
  Sum(C & elements) {
    typename emp::remove_ptr_type<typename C::value_type>::type total = 0;
    for (auto element : elements) {
      if constexpr (emp::is_ptr_type<typename C::value_type>::value) total += *element;
      else total += element;
    }
    return total;
  }

  /// Sum the RESULTS of scalar values in a container; if pointers to scalars, convert to scalar type
  template <typename C, typename FUN>
  auto SumScalarResults(C & elements, FUN && fun) {
    using return_t = decltype( fun( emp::remove_ptr_value(*elements.begin()) ) );
    return_t total = 0;
    for (auto element : elements) {
      total += fun(emp::remove_ptr_value(element));
    }
    return total;
  }

  /// Calculate Shannon Entropy of the members of the container passed
  template <typename C>
  double ShannonEntropy(C & elements) {
    // Count number of each value present
    emp::map<typename emp::remove_ptr_type<typename C::value_type>::type, int> counts;
    for (auto element : elements) {
      // If we have a container of pointers, dereference them
      if constexpr (emp::is_ptr_type<typename C::value_type>::value) {
        auto it = counts.find(*element);
        if (it != counts.end()) it->second++;
        else counts[*element] = 1;
      }
      // otherwise operate on elements directly.
      else {
        auto it = counts.find(element);
        if (it != counts.end()) it->second++;
        else counts[element] = 1;
      }
    }

    // Shannon entropy calculation
    double result = 0;
    for (auto & element : counts) {
      double p = double(element.second)/elements.size();
      result +=  p * Log2(p);
    }

    return -1 * result;
  }


  /// Calculate the mean of the values in a container
  /// If values are pointers, they will be automatically de-referenced
  /// Values must be numeric.
  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type>
  Mean(C & elements) {
    return (double)Sum(elements)/ (double) elements.size();
  }

  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type>
  Median(C elements) {
    emp::Sort(elements);
    if (elements.size() % 2 == 1) {
      return elements[elements.size() / 2];
    } else {
      return (elements[elements.size() / 2 - 1] + elements[elements.size() / 2])/2.0;
    }
  }


  /// Calculate variance of the members of the container passed
  /// Only works on containers with a scalar member type
  template <typename C>
  auto Variance(C & elements) {
    const double mean = Mean(elements);
    auto sum = SumScalarResults(elements,
                                [mean](auto x){ return emp::Pow(mean - (double) x, 2); } );
    return sum / (elements.size() - 1);
  }


  /// Calculate the standard deviation of the values in a container
  /// If values are pointers, they will be automatically de-referenced
  /// Values must be numeric.
  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type>
  StandardDeviation(C & elements) {
    return sqrt(Variance(elements));
  }

  /// Calculate the standard error of the values in a container
  /// If values are pointers, they will be automatically de-referenced
  /// Values must be numeric.
  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type>
  StandardError(C & elements) {
    return StandardDeviation(elements) / sqrt(elements.size());
  }

  /// Count the number of unique elements in a container
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value, int>::type
  UniqueCount(C & elements) {
    // Converting to a set will remove duplicates leaving only unique values
    std::set<typename C::value_type> unique_elements(elements.begin(),
                           elements.end());
    return unique_elements.size();
  }
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  /// Count the number of unique elements in the container of pointers. (compares objects pointed
  /// to; pointers do not have to be identical)
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value, int>::type
  UniqueCount(C & elements) {
    // Converting to a set will remove duplicates leaving only unique values
    using pointed_at = typename emp::remove_ptr_type<typename C::value_type>::type;
    std::set<pointed_at> unique_elements;
    for (auto element : elements) {
        unique_elements.insert(*element);
    }

    return unique_elements.size();
  }
#endif
  /// Run the provided function on every member of a container and return the MAXIMUM result.
  template <typename C, typename RET_TYPE, typename ARG_TYPE>
  RET_TYPE MaxResult(std::function<RET_TYPE(ARG_TYPE)> & fun, C & elements){
    auto best = fun(elements.front());  // @CAO Technically, front is processed twice...
    for (auto element : elements){
      auto result = fun(element);
      if (result > best) best = result;
    }
    return best;
  }

  /// Run the provided function on every member of a container and return the MINIMUM result.
  template <typename C, typename RET_TYPE, typename ARG_TYPE>
  RET_TYPE MinResult(std::function<RET_TYPE(ARG_TYPE)> & fun, C & elements){
    auto best = fun(elements.front());  // @CAO Technically, front is processed twice...
    for (auto element : elements){
      auto result = fun(element);
      if (result < best) best = result;
    }
    return best;
  }

  /// Run the provided function on every member of a container and return the AVERAGE result.
  /// Function must return a scalar (i.e. numeric) type.
  template <typename C, typename RET_TYPE, typename ARG_TYPE>
  typename std::enable_if<std::is_scalar<RET_TYPE>::value, double>::type
  MeanResult(std::function<RET_TYPE(ARG_TYPE)> & fun, C & elements){
    double cumulative_value = 0;
    double count = 0;
    for (auto element : elements){
        ++count;
        cumulative_value += fun(element);
    }
    return (cumulative_value / count);
  }

  /// Run the provided function on every member of a container and return a vector of ALL results.
  template <typename C, typename RET_TYPE, typename ARG_TYPE>
  emp::vector<RET_TYPE> ApplyFunction(std::function<RET_TYPE(ARG_TYPE)> & fun, C & elements) {
      emp::vector<RET_TYPE> results;
      for (auto element : elements){
          results.push_back(fun(element));
      }
      return results;
  }

  // This variant is actually super confusing because the value_type of world
  // and population managers are pointers whereas the type that they're templated
  // on is not. Also because the insert method for emp::vectors doesn't take an
  // additional argument?

  /* template <template <typename> class C, typename RET_TYPE, typename T>
  C<RET_TYPE> RunFunctionOnContainer(std::function<RET_TYPE(T)> fun, C<T> & elements) {
      C<RET_TYPE> results;
      for (auto element : elements){
          results.insert(fun(element), results.back());
      }
      return results;
  } */

}

//Base class outputs most recent
//Write derived class

#endif // #ifndef EMP_MATH_STATS_HPP_INCLUDE
