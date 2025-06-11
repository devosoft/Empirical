/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2024 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/in_progress/bimap.hpp
 * @brief A two-way unordered map; simply two maps linked together.
 * @note Status: ALPHA
 *
 */

#pragma once

#ifndef INCLUDE_EMP_IN_PROGRESS_BIMAP_HPP_GUARD
#define INCLUDE_EMP_IN_PROGRESS_BIMAP_HPP_GUARD


#include <unordered_map>

#include "../base/assert.hpp"

namespace emp {

  template <typename T1, typename T2>
  class bimap {
  private:
    std::unordered_map<T1,T2> map1;
    std::unordered_map<T2,T1> map2;

  public:
    void Insert(const T1 & in1, const T2 & in2) {
      map1.insert({in1,in2});
      map2.insert({in2,in1});
    }

    void Set(const T1 & in1, const T2 & in2) { Insert(in1, in2); }
    void Set2(const T2 & in2, const T1 & in1) { Insert(in1, in2); }

    T2 & Get1(const T1 & in1)
  };

}

#endif // #ifndef EMP_DATASTRUCTS_BIMAP_HPP_INCLUDE

// Local settings for Empecable file checker.
// empecable_words: bimap
