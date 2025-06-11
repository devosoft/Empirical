/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2021 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/bits/BitSet.hpp
 * @brief A drop-in replacement for std::bitset, with additional bit magic features; aliases BitArray.
 * Status: RELEASE
 *
 */

#pragma once

#ifndef INCLUDE_EMP_BITS_BIT_SET_HPP_GUARD
#define INCLUDE_EMP_BITS_BIT_SET_HPP_GUARD

#include <stddef.h>

#include "BitArray.hpp"
#include "Bits.hpp"  // New version of BitSet is in Bits.hpp

namespace emp::old {

  template <size_t NUM_BITS>
  using BitSet = emp::old::BitArray<NUM_BITS, false>;

}

#endif  // #ifndef INCLUDE_EMP_BITS_BIT_SET_HPP_GUARD
