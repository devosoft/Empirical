/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/RobinHoodMap.hpp
 * @brief Robin Hood hashing map — minimize how "wrong" a position can be in a hash table.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD

#include <compare>
#include <type_traits>
#include <utility>

#include "../meta/type_traits.hpp"
#include "RobinHoodBase.hpp"

namespace emp {

  /// Entry type for RobinHoodMap: stores a key and a mapped value.
  template <typename KEY_T, typename MAPPED_T>
  struct RHMapEntry {
    KEY_T    first;
    MAPPED_T second;

    [[nodiscard]] KEY_T    & Key()         { return first;  }
    [[nodiscard]] const KEY_T & Key() const { return first;  }
    [[nodiscard]] MAPPED_T & Value()        { return second; }
    [[nodiscard]] const MAPPED_T & Value() const { return second; }

    // ToRef is used by the iterator to produce its dereference result.
    // For a map, non-const iterators expose a mutable Entry reference.
    static RHMapEntry &       ToRef(RHMapEntry & e)       noexcept { return e; }
    static const RHMapEntry & ToRef(const RHMapEntry & e) noexcept { return e; }

    operator std::pair<const KEY_T, MAPPED_T>() const { return {first, second}; }

    using value_type = std::pair<const KEY_T, MAPPED_T>;
  };

  // =========================================================================

  template <typename KEY_T, typename MAPPED_T, bool IMPROVE_HASH = false>
  class RobinHoodMap
    : public RobinHoodBase<KEY_T, RHMapEntry<KEY_T, MAPPED_T>, IMPROVE_HASH>
  {
    using Base  = RobinHoodBase<KEY_T, RHMapEntry<KEY_T, MAPPED_T>, IMPROVE_HASH>;
    using Entry = RHMapEntry<KEY_T, MAPPED_T>;

  public:
    using iterator        = typename Base::iterator;
    using const_iterator  = typename Base::const_iterator;
    using key_type        = KEY_T;
    using mapped_type     = MAPPED_T;
    using value_type      = std::pair<const KEY_T, MAPPED_T>;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;

    RobinHoodMap() = default;
    RobinHoodMap(const RobinHoodMap &)            = default;
    RobinHoodMap(RobinHoodMap &&) noexcept        = default;
    RobinHoodMap & operator=(const RobinHoodMap &) = default;
    RobinHoodMap & operator=(RobinHoodMap &&) noexcept = default;
    ~RobinHoodMap() = default;

    std::pair<iterator, bool> insert(const std::pair<const KEY_T, MAPPED_T> & in) {
      return Insert(in.first, in.second);
    }

    /// Insert with unique-key semantics.
    /// @return {iterator-to-key, was-newly-inserted}
    std::pair<iterator, bool> Insert(const KEY_T & key, const MAPPED_T & value) {
      emp_assert(Base::OK());
      auto [pos, is_new] = Base::InsertEntry(Entry{key, value});
      return {iterator{this, pos}, is_new};
    }

    /// Construct a key-value pair in-place from forwarded arguments.
    /// Arguments are forwarded to construct std::pair<const KEY_T, MAPPED_T>.
    /// @return {iterator-to-key, was-newly-inserted}
    template <typename... ARGS>
    std::pair<iterator, bool> emplace(ARGS &&... args) {
      value_type kv(std::forward<ARGS>(args)...);
      auto [pos, is_new] = Base::InsertEntry(Entry{std::move(kv.first), std::move(kv.second)});
      return {iterator{this, pos}, is_new};
    }

    // === VALUE ACCESS ===

    template <class Self>
    [[nodiscard]] auto FindPtr(this Self & self, const KEY_T & key)
      -> std::add_pointer_t<emp::match_const_t<MAPPED_T, Self>>
    {
      if (self.table.empty()) return nullptr;
      auto sp = self.MakeSearchPos(key);
      while (sp.dist <= self.occupied[sp]) {
        if (self.TestAt(sp, key)) return &self.table[sp].Value();
        sp.Next(self.table_mask);
      }
      return nullptr;
    }

    /// Look up a key; returns the mapped value or a default if not found.
    [[nodiscard]] MAPPED_T FindValue(const KEY_T & key, MAPPED_T default_obj = {}) const {
      const MAPPED_T * ptr = FindPtr(key);
      return ptr ? *ptr : default_obj;
    }

    MAPPED_T & operator[](const KEY_T & key) {
      MAPPED_T * val = FindPtr(key);
      if (val) return *val;
      Base::InsertEntry(Entry{key, MAPPED_T{}});
      return *FindPtr(key);
    }

    /// Retrieve value for a key that is guaranteed to be present (asserts in debug mode).
    [[nodiscard]] const MAPPED_T & Get(const KEY_T & key) const {
      const MAPPED_T * val = FindPtr(key);
      emp_assert(val);
      return *val;
    }

    [[nodiscard]] MAPPED_T & Get(const KEY_T & key) {
      MAPPED_T * val = FindPtr(key);
      emp_assert(val);
      return *val;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_ROBIN_HOOD_MAP_HPP_GUARD
