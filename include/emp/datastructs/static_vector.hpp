/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025.
*/
/**
 *  @file
 *  @brief A simple replacement for std::vector when maximum size is known at compile time.
 *  @note Status: ALPHA
 */

#ifndef EMP_BASE_STATIC_VECTOR_HPP_INCLUDE
#define EMP_BASE_STATIC_VECTOR_HPP_INCLUDE


#include <initializer_list>
#include <iterator>
#include <stddef.h>
#include <utility>
#include <vector>

#include "assert.hpp"

namespace emp {

  template <typename T, size_t MAX_SIZE>
  class StaticVector {
  private:
    std::array<T, MAX_SIZE> values{};
    size_t cur_size = 0;

    using this_t = StaticVector<T, MAX_SIZE>;
  public:
    using value_type = T;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    auto operator<=>(const this_t &) const = default;

    T & operator[](size_t pos) { return values[pos]; }
    const T & operator[](size_t pos) const { return values[pos]; }

    auto begin() noexcept { return values.begin(); }
    auto begin() const noexcept { return values.begin(); }
    auto end() noexcept { return cur_size < MAX_SIZE ? values.begin() + cur_size : values.end(); }
    auto end() const noexcept { return cur_size < MAX_SIZE ? values.begin() + cur_size : values.end(); }

    size_t size() const { return cur_size; }

    this_t & resize(size_t new_size) {
      emp_assert(new_size <= MAX_SIZE);
      cur_size = new_size;
      return *this;
    }

    this_t & resize(size_t new_size, const T & default_value) {
      for (size_t i = cur_size; i < new_size; ++i) {
        values[i] = default_value;
      }
      return resize(new_size);
    }

    T & back() { emp_assert(cur_size > 0); return values[cur_size-1]; }
    const T & back() const { emp_assert(cur_size > 0); return values[cur_size-1]; }
    T & front() { emp_assert(cur_size > 0); return values[0]; }
    const T & front() const { emp_assert(cur_size > 0); return values[0]; }

    void fill(const T & val) {
      for (size_t i = 0; i < cur_size; ++i) values[i] = val;
    }

    template <typename... PB_Ts>
    void push_back(PB_Ts &&... /* args */) { emp_assert(false, "invalid operation for array!"); }
    void pop_back() { emp_assert(false, "invalid operation for array!"); }

    template <typename... ARGS>
    iterator insert(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    iterator erase(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    iterator emplace(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    void emplace_back(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
    }

  };

}

#endif // #ifndef EMP_BASE_STATIC_VECTOR_HPP_INCLUDE
