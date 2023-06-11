/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file optional.hpp
 *  @brief Audited implementation of std::optional.
 *  @note Status: RELEASE
 *
 *  Drop-in replacements for std::optional.
 *  In debug mode, operator * and operator-> value accesses are checked for undefined behavior.
 */

#ifndef EMP_BASE_OPTIONAL_HPP_INCLUDE
#define EMP_BASE_OPTIONAL_HPP_INCLUDE

#include <optional>
#include <type_traits>
#include <utility>

#include "assert.hpp"

#ifdef EMP_NDEBUG

namespace emp {
  template <typename T> using optional = std::optional<T>;

  template <class T>
  inline auto make_optional(T&& val) {
    return std::make_optional<T>(std::forward<T>(val));
  }

  template <class T, class... Args>
  inline auto make_optional(Args&&... args) {
    return std::make_optional<T>(std::forward<Args>(args)... );
  }

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

    constexpr const T* operator->() const { return &parent_t::value(); }

    constexpr T* operator->() { return &parent_t::value(); }

    constexpr const T& operator*() const& { return parent_t::value(); }

    constexpr T& operator*() & { return parent_t::value(); }

    constexpr T&& operator*() && { return std::move( parent_t::value() ); }

    constexpr const T&& operator*() const&& {
      return std::move( parent_t::value() );
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

namespace std {

  template <typename T>
  struct hash<emp::optional<T>> {
    size_t operator()( const emp::optional<T>& opt ) const {
      return std::hash<std::optional<T>>{}( opt );
    }
  };

} // namespace std

#endif

#endif // #ifndef EMP_BASE_OPTIONAL_HPP_INCLUDE
