/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file optional.h
 *  @brief Audited implementation of std::optional.
 *  @note Status: RELEASE
 *
 *  In release mode, functions identically to std::optional.
 *  In debug mode, operator * and operator-> value accesses are
 *  checked for undefined behavior.
 */

#ifndef EMP_OPTIONAL_H
#define EMP_OPTIONAL_H

#include <type_traits>
#include <utility>

#include "assert.h"

#ifdef EMP_NDEBUG

namespace emp {
  template <typename T> using optional = std::optional<T>;
}

#else // #EMP_NDEBUG *not* set

namespace emp {

  template<class T>
  class optional : public std::optional<T> {

    using parent_t = std::optional<T>;

  public:

    // inherit parent's constructors
    // adapted from https://stackoverflow.com/a/434784
    using parent_t::parent_t;

    constexpr const T* operator->() const {
      emp_assert(parent_t::has_value(), "bad optional access");
      return parent_t::operator->();
    }

    constexpr T* operator->() {
      emp_assert(parent_t::has_value(), "bad optional access");
      return parent_t::operator->();
    }

    constexpr const T& operator*() const& {
      emp_assert(parent_t::has_value(), "bad optional access");
      return parent_t::operator*();
    }

    constexpr T& operator*() & {
      emp_assert(parent_t::has_value(), "bad optional access");
      return parent_t::operator*();
    }

    constexpr T&& operator*() && {
      emp_assert(parent_t::has_value(), "bad optional access");
      return std::move( parent_t::operator*() );
    }

    constexpr const T&& operator*() const&& {
      emp_assert(parent_t::has_value(), "bad optional access");
      return std::move( parent_t::operator*() );
    }

  };

  template <class T>
  constexpr emp::optional<std::decay_t<T>>
  make_optional(T&& value) {
    return emp::optional<std::decay_t<T>>{
      std::forward<T>(value)
    };
  }

  template <class T, class... Args>
  constexpr emp::optional<T>
  make_optional(Args&&... args) {
    return emp::optional<T>{
      std::in_place,
      std::forward<Args>(args)...
    };
  }

  template <class T, class Elem, class... Args>
  constexpr emp::optional<T> make_optional(
    std::initializer_list<Elem> il,
    Args&&... args
  ) {
    return emp::optional<T>{
      std::in_place,
      il,
      std::forward<Args>(args)...
    };
  }

} // namespace emp

#endif

#endif
