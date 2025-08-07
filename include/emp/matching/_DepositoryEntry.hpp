/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2020 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/matching/_DepositoryEntry.hpp
 * @brief Helper struct for MatchDepository.
 *
 */

#pragma once

#ifndef INCLUDE_EMP_MATCHING_DEPOSITORY_ENTRY_HPP_impl_GUARD
#define INCLUDE_EMP_MATCHING_DEPOSITORY_ENTRY_HPP_impl_GUARD

namespace emp {
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {

    template <typename Val, typename Tag, typename Regulator>
    struct DepositoryEntry {
      Regulator reg;
      Val val;
      Tag tag;

      DepositoryEntry(const Val & val_, const Tag & tag_) : val(val_), tag(tag_) { ; }
    };

  }  // namespace internal
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS
}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_MATCHING_DEPOSITORY_ENTRY_HPP_impl_GUARD
