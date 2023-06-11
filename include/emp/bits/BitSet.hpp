/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2021.
 *
 *  @file BitSet.hpp
 *  @brief A drop-in replacement for std::bitset, with additional bit magic features; aliases BitArray.
 *  @note Status: RELEASE
 *
 */

#ifndef EMP_BITS_BITSET_HPP_INCLUDE
#define EMP_BITS_BITSET_HPP_INCLUDE


#include "BitArray.hpp"

namespace emp {

  template <size_t NUM_BITS>
  using BitSet = emp::BitArray<NUM_BITS, false>;

}

#endif // #ifndef EMP_BITS_BITSET_HPP_INCLUDE
