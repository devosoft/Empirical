/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/RobinHoodSet.hpp
 * @brief Robin Hood hashing set — same probing strategy as RobinHoodMap, keys only.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_SET_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_SET_HPP_GUARD

#include <utility>

#include "RobinHoodBase.hpp"

namespace emp {

  /// Entry type for RobinHoodSet: stores only a key; iterator always gives const KEY_T &.
  template <typename KEY_T>
  struct RHSetEntry {
    KEY_T first;

    [[nodiscard]] KEY_T    & Key()          { return first; }
    [[nodiscard]] const KEY_T & Key() const { return first; }

    // Set iterators always yield a const reference — keys must not be mutated.
    static const KEY_T & ToRef(const RHSetEntry & e) noexcept { return e.first; }
    static const KEY_T & ToRef(RHSetEntry &       e) noexcept { return e.first; }

    using value_type = KEY_T;
  };

  // =========================================================================

  template <typename KEY_T, bool IMPROVE_HASH = false>
  class RobinHoodSet
    : public RobinHoodBase<KEY_T, RHSetEntry<KEY_T>, IMPROVE_HASH>
  {
    using Base  = RobinHoodBase<KEY_T, RHSetEntry<KEY_T>, IMPROVE_HASH>;
    using Entry = RHSetEntry<KEY_T>;

  public:
    using iterator        = typename Base::iterator;
    using const_iterator  = typename Base::const_iterator;
    using key_type        = KEY_T;
    using value_type      = KEY_T;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

    RobinHoodSet() = default;
    RobinHoodSet(const RobinHoodSet &)            = default;
    RobinHoodSet(RobinHoodSet &&) noexcept        = default;
    RobinHoodSet & operator=(const RobinHoodSet &) = default;
    RobinHoodSet & operator=(RobinHoodSet &&) noexcept = default;
    ~RobinHoodSet() = default;

    /// Construct from an initializer list.
    RobinHoodSet(std::initializer_list<KEY_T> init) {
      Base::reserve(init.size());
      for (const KEY_T & k : init) Insert(k);
    }

    /// Insert a key.  Returns {iterator-to-key, was-newly-inserted}.
    std::pair<iterator, bool> insert(const KEY_T & key) { return Insert(key); }
    std::pair<iterator, bool> Insert(const KEY_T & key) {
      emp_assert(Base::OK());
      auto [pos, is_new] = Base::InsertEntry(Entry{key});
      return {iterator{this, pos}, is_new};
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_SET_HPP_GUARD
