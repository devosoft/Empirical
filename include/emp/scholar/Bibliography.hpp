/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2017 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/scholar/Bibliography.hpp
 * @brief A collection of references.
 */

#pragma once

#ifndef INCLUDE_EMP_SCHOLAR_BIBLIOGRAPHY_HPP_GUARD
#define INCLUDE_EMP_SCHOLAR_BIBLIOGRAPHY_HPP_GUARD

#include "../base/vector.hpp"

#include "Citation.hpp"

namespace emp {

  class Bibliography {
  private:
    emp::vector<Citation> refs;
    std::set<std::string> keywords;
  public:
    Bibliography() { ; }

    ~Bibliography() { ; }

    void AddRef(const Citation & in_ref) { refs.push_back(in_ref); }
  };

};  // namespace emp

#endif  // #ifndef INCLUDE_EMP_SCHOLAR_BIBLIOGRAPHY_HPP_GUARD
