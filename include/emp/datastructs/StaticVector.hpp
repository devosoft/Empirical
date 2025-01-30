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

#ifndef EMP_DATASTRUCT_STATIC_VECTOR_HPP_INCLUDE
#define EMP_DATASTRUCT_STATIC_VECTOR_HPP_INCLUDE


#include <initializer_list>
#include <iterator>
#include <stddef.h>
#include <utility>
#include <vector>

#include "base/assert.hpp"
#include "base/array.hpp"

namespace emp {

  template <typename VALUE_T, size_t MAX_SIZE>
  class StaticVector {
  private:
    std::array<VALUE_T, MAX_SIZE> values{};
    size_t cur_size = 0;

    using this_t = StaticVector<VALUE_T, MAX_SIZE>;

    // Move a chunk of vector from one place to another.
    void RawMove(size_t from_id, size_t to_id, size_t count) {
      assert(from_id+count <= MAX_SIZE);
      assert(to_id+count <= MAX_SIZE);
      // Move current values out of the way.
      auto start = values.begin()+from_id;
      auto stop = start+count;
      auto target = values.begin()+to_id;
      if (from_id < to_id) std::move_backward(start, stop, target+count);
      else std::move(start, stop, target);
    }
  public:
    using value_type = VALUE_T;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    auto operator<=>(const this_t &) const = default;

    VALUE_T & operator[](size_t pos) { return values[pos]; }
    const VALUE_T & operator[](size_t pos) const { return values[pos]; }

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

    this_t & resize(size_t new_size, const VALUE_T & default_value) {
      emp_assert(new_size <= MAX_SIZE);
      for (size_t i = cur_size; i < new_size; ++i) {
        values[i] = default_value;
      }
      return resize(new_size);
    }

    VALUE_T & back() { emp_assert(cur_size > 0); return values[cur_size-1]; }
    const VALUE_T & back() const { emp_assert(cur_size > 0); return values[cur_size-1]; }
    VALUE_T & front() { emp_assert(cur_size > 0); return values[0]; }
    const VALUE_T & front() const { emp_assert(cur_size > 0); return values[0]; }

    void fill(const VALUE_T & val) {
      for (size_t i = 0; i < cur_size; ++i) values[i] = val;
    }

    // Fill vector with a provided value.
    // 1 arg -> specify value only; fill entire vector.
    // 2 args -> also set starting point for fill; fill to end of vector.
    // 3 args -> also set count for fill.
    template <typename T>
    void Fill(T && value, size_t start, size_t count) {
      while (--count > 0) values[start++] = value;
      // Move last (or only!) value by move, if applicable.
      values[start] = std::forward<T>(value);
    }

    // Allow fill to have optional arguments.
    template <typename T>
    void Fill(T && value, size_t start=0) {
      Fill(std::forward<T>(value, start, cur_size-start));
    }

    template <typename T>
    void Push(T && value, size_t count=1) {
      emp_assert(cur_size + count <= MAX_SIZE);
      Fill(std::forward<T>(value), cur_size, count);
      cur_size += count;
    }
    VALUE_T && Pop() {
      emp_assert(cur_size > 0);
      --cur_size;
      return std::move(values[cur_size]);
    }

    template <typename T>
    void Insert(size_t pos, T && value, size_t count=1) {
      emp_assert(cur_size + count <= MAX_SIZE);
      emp_assert(pos <= cur_size);

      // If we are inserting at the end, use push.
      if (!count) return; // Nothing to insert.
      if (pos == cur_size) return Push(std::forward<T>(value), count);

      RawMove(pos, cur_size, count);             // Move current values out of the way.
      Fill(std::forward<T>(value), pos, count);  // Insert new values.
    }

    template <typename T>
    void Erase(size_t pos, size_t count=1) {
      if (!count) return; // Nothing to erase.
      const size_t end_pos = pos+count;
      if (end_pos == cur_size) {
        cur_size -= count;
        return;
      }
      const size_t move_size = cur_size-end_pos;
      RawMove(end_pos, pos, move_size);
      cur_size = pos + move_size;
    }

  };

}

#endif // #ifndef EMP_DATASTRUCT_STATIC_VECTOR_HPP_INCLUDE
