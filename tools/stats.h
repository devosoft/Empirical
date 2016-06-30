//  This file is part of Empirical, https://github.com/devosoft/Empirical/
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE

//  This file contains functions for calculating various statistics about
//  a population.
//

#ifndef EMP_STATS_H
#define EMP_STATS_H

#include <type_traits>
#include <map>
#include <unordered_map>
#include <cmath>
#include <vector>
#include <set>
#include <unordered_set>
#include <functional>

#include "../tools/vector.h"

namespace emp{
namespace evo{

  //Calculates Shannon Entropy of the members of the container passed
  template <typename C>
  typename std::enable_if<!std::is_pointer<typename C::value_type>::value, double>::type
  ShannonEntropy(C & elements) {

    //Count number of each value present
    std::map<typename C::value_type, int> counts;
    for (auto element : elements) {
      if (counts.find(element) != counts.end()) {
	       counts[element]++;
      } else {
	       counts[element] = 1;
      }
    }

    //Shannon entropy calculation
    double result = 0;
    for (auto element : counts) {
      double p = double(element.second)/elements.size();
      result +=  p * log2(p);
    }

    return -1 * result;
  }

  //Calculates Shannon Entropy of the members of the container when those members
  //are pointers
  template <typename C>
  typename std::enable_if<std::is_pointer<typename C::value_type>::value, double>::type
  ShannonEntropy(C & elements) {

    using pointed_at = typename std::remove_pointer<typename C::value_type>::type;
    //Count number of each value present
    std::map<pointed_at, int> counts;
    for (auto element : elements) {
      if (counts.find(*element) != counts.end()) {
        counts[*element]++;
      } else {
        counts[*element] = 1;
      }

    }
    //Shannon entropy calculation
    double result = 0;
    for (auto element : counts) {
      double p = double(element.second)/elements.GetSize();
      result +=  p * log2(p);
    }

    return -1 * result;
  }

  //Calculates number of unique elements in the container passed
  template <typename C>
  typename std::enable_if<!std::is_pointer<typename C::value_type>::value, int>::type
  UniqueCount(C & elements) {
    //Converting to a set will remove duplicates leaving only unique values
    std::set<typename C::value_type> unique_elements(elements.begin(),
                           elements.end());
    return unique_elements.size();
  }

  //Calculates number of unique elements in the container of pointers passed
  template <typename C>
  typename std::enable_if<std::is_pointer<typename C::value_type>::value, int>::type
  UniqueCount(C & elements) {
    //Converting to a set will remove duplicates leaving only unique values
    using pointed_at = typename std::remove_pointer<typename C::value_type>::type;
    std::set<pointed_at> unique_elements;
    for (auto element : elements) {
        unique_elements.insert(*element);
    }

    return unique_elements.size();
  }

  //Takes a function and a container of items that that function can be run on
  //and returns the maximum value
  template <typename C, typename RET_TYPE>

  RET_TYPE MaxFunctionReturn(std::function<RET_TYPE(typename C::value_type)> & fun, C & elements){
    double highest = 0;
    for (auto element : elements){
      double result = fun(element);
      if (result > highest){
        highest = result;
      }
    }
    return highest;
  }

  //Takes a function and a container of items that that function can be run on
  //and returns the average value. Function must return a double.
  template <typename C>
  double AverageFunctionReturn(std::function<double(typename C::value_type)> & fun, C & elements){
    double cumulative_value = 0;
    double count = 0;
    for (auto element : elements){
        ++count;
        cumulative_value += fun(element);
    }
    return (cumulative_value / count);
  }

  template <typename C, typename RET_TYPE>
  emp::vector<RET_TYPE> RunFunctionOnContainer(std::function<RET_TYPE(typename C::value_type)> & fun, C & elements) {
      emp::vector<RET_TYPE> results;
      for (auto element : elements){
          results.push_back(fun(element));
      }
      return results;
  }

  //This variant is actually super confusing because the value_type of world
  //and population managers are pointers whereas the type that they're templated
  //on is not. Also because the insert method for emp::vectors doesn't take an
  //additional argument?
  /*template <template <typename> class C, typename RET_TYPE, typename T>
  C<RET_TYPE> RunFunctionOnContainer(std::function<RET_TYPE(T)> fun, C<T> & elements) {
      C<RET_TYPE> results;
      for (auto element : elements){
          results.insert(fun(element), results.back());
      }
      return results;
  }*/

}
}

#endif

//Base class outputs most recent
//Write derived class
