/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2019-2024
*/
/**
 *  @file
 *  @brief Simple functions to manipulate values.
 *  @note Status: ALPHA
 */

#ifndef EMP_TOOLS_VALUE_UTILS_HPP_INCLUDE
#define EMP_TOOLS_VALUE_UTILS_HPP_INCLUDE

#include <string>
#include "String.hpp"

namespace emp {

  template <typename T>
  inline double ToDouble(const T & in) { return (double) in; }

  template <>
  inline double ToDouble<std::string>(const std::string & in) {
    return std::stod(in);
  }

  template <>
  inline double ToDouble<emp::String>(const emp::String & in) {
    return in.AsDouble();
  }

  template <typename T>
  inline T FromDouble(double in) { return (T) in; }

  template <>
  inline std::string FromDouble<std::string>(double in) {
    return std::to_string(in);
  }

  template <>
  inline emp::String FromDouble<emp::String>(double in) {
    return emp::MakeString(in);
  }

}

#endif // #ifndef EMP_TOOLS_VALUE_UTILS_HPP_INCLUDE
