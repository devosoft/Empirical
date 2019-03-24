/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2019
 *
 *  @file stats.h
 *  @brief Functions for calculating various statistics about an ensemble.
 *  @note Status: BETA
 */


#ifndef EMP_STATS_H
#define EMP_STATS_H

#include <type_traits>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <set>
#include <unordered_set>
#include <functional>

#include "../base/map.h"
#include "../base/vector.h"
#include "../meta/type_traits.h"
#include "math.h"

namespace emp {

  /// Calculate sum of the members of the container passed
  /// Only works on containers with a scalar member type
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value && std::is_scalar<typename C::value_type>::value, typename C::value_type>::type
  Sum(C & elements) {

    double total = 0;
    for (auto element : elements) {
      total += element;
    }

    return total;
  }

  /// Calculate sum of the values pointed at by pointers in a container
  /// Only works on containers of pointers to a scalar type
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value && std::is_scalar<typename emp::remove_ptr_type<typename C::value_type>::type >::value, typename emp::remove_ptr_type<typename C::value_type>::type >::type
  Sum(C & elements) {

    double total = 0;
    for (auto element : elements) {
      total += *element;
    }

    return total;
  }


  /// Calculate Shannon Entropy of the members of the container passed
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value, double>::type
  ShannonEntropy(C & elements) {

    // Count number of each value present
    emp::map<typename C::value_type, int> counts;
    for (auto element : elements) {
      if (counts.find(element) != counts.end()) {
	       counts[element]++;
      } else {
	       counts[element] = 1;
      }
    }

    // Shannon entropy calculation
    double result = 0;
    for (auto element : counts) {
      double p = double(element.second)/elements.size();
      result +=  p * Log2(p);
    }

    return -1 * result;
  }

  /// Calculate Shannon Entropy of the members of the container when those members are pointers
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value, double>::type
  ShannonEntropy(C & elements) {
    //   std::cout<< "In se" << std::endl;
    using pointed_at = typename emp::remove_ptr_type<typename C::value_type>::type;
    // Count number of each value present
    emp::map<pointed_at, int> counts;
    for (auto element : elements) {
      if (counts.find(*element) != counts.end()) {
        counts[*element]++;
      } else {
        counts[*element] = 1;
      }

    }
    // Shannon entropy calculation
    double result = 0;
    for (auto element : counts) {
      double p = double(element.second)/elements.size();
      result +=  p * log2(p);
    }
    //   std::cout<< "leaving se" << std::endl;
    return -1 * result;
  }

  /// Calculate variance of the members of the container passed
  /// Only works on containers with a scalar member type
  template <typename C>
  typename std::enable_if<!emp::is_ptr_type<typename C::value_type>::value && std::is_scalar<typename C::value_type>::value, double>::type
  Variance(C & elements) {

    double var = 0;
    double mean = (double)Sum(elements)/elements.size();
    for (auto element : elements) {
      var += emp::Pow(element - mean, 2);
    }
    return var/elements.size();
  }

  /// Calculate variance of the values pointed at by members of the container passed
  /// Only works on containers with a scalar member type
  template <typename C>
  typename std::enable_if<emp::is_ptr_type<typename C::value_type>::value && std::is_scalar<typename emp::remove_ptr_type<typename C::value_type>::type >::value, double>::type
  Variance(C & elements) {

    double var = 0;
    double mean = (double)Sum(elements)/elements.size();
    for (auto element : elements) {
      var += emp::Pow(*element - mean, 2);
    }
    return var/elements.size();
  }

  /// Calculate the mean of the values in a container
  /// If values are pointers, they will be automatically de-referenced
  /// Values must be numeric.
  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type> 
  Mean(C & elements) {
    return (double)Sum(elements)/elements.size();
  }

  /// Calculate the standard deviation of the values in a container
  /// If values are pointers, they will be automatically de-referenced
  /// Values must be numeric.
  template <typename C>
  emp::sfinae_decoy<double, typename C::value_type> 
  StandardDeviation(C & elements) {
    return sqrt(Variance(elements));
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

#endif

//Base class outputs most recent
//Write derived class
