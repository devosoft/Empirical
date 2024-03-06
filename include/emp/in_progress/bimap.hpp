/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024
 *
 *  @file Bimap.hpp
 *  @brief A two-way unordered map; simply two maps linked together.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_DATASTRUCTS_BIMAP_HPP_INCLUDE
#define EMP_DATASTRUCTS_BIMAP_HPP_INCLUDE


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
