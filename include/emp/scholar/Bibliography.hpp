/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2016-2017
*/
/**
 *  @file
 *  @brief A collection of references.
 */

#ifndef EMP_SCHOLAR_BIBLIOGRAPHY_HPP_INCLUDE
#define EMP_SCHOLAR_BIBLIOGRAPHY_HPP_INCLUDE

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

    void AddRef(const Citation & in_ref) {
      refs.push_back(in_ref);
    }
  };

};

#endif // #ifndef EMP_SCHOLAR_BIBLIOGRAPHY_HPP_INCLUDE
