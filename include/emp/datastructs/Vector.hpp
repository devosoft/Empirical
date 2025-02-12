/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2025.
*/
/**
 *  @file
 *  @brief A scaled-up version of std::vector with additional functionality.
 *  @note Status: ALPHA
 */

#ifndef EMP_DATASTRUCT_VECTOR_HPP_INCLUDE
#define EMP_DATASTRUCT_VECTOR_HPP_INCLUDE

#include <initializer_list>
#include <iterator>
#include <stddef.h>
#include <utility>
#include <vector>

#include "../base/vector.hpp"

#include "StaticVector.hpp"

namespace emp {

  /**
   * @brief A class that can be either a static or dynamic vector, with extra functionality.
   * 
   * @tparam VALUE_T Value type that the vector will be using.
   * @tparam MAX_SIZE Maximum number of entries in vector (use 0 for unlimited)
   */
  template <typename VALUE_T, size_t MAX_SIZE=0>
  class Vector {
  private:
    static constexpr bool IS_STATIC = !MAX_SIZE;
    using this_t = Vector<VALUE_T, MAX_SIZE>;
    using static_t = StaticVector<VALUE_T, MAX_SIZE>;
    using dynamic_t = emp::vector<VALUE_T>;
    using vec_t = std::conditional_t<IS_STATIC, dynamic_t, static_t>;
    vec_t values;

  public:
    using value_type = VALUE_T;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    Vector() = default;
    Vector(const Vector &) = default;
    Vector(Vector &&) = default;
    Vector(size_t count, const VALUE_T & value=VALUE_T()) : values(count, value) { }
    Vector(std::initializer_list<VALUE_T> init) : values(init) { }
    ~Vector() = default;

    auto operator<=>(const this_t &) const = default;

    VALUE_T & operator[](size_t pos) { return values[pos]; }
    const VALUE_T & operator[](size_t pos) const { return values[pos]; }

    size_t size() const { return values.size(); }

    auto begin() noexcept { return values.begin(); }
    auto begin() const noexcept { return values.begin(); }
    auto end() noexcept { values.end(); }
    auto end() const noexcept { values.end(); }

    this_t & Resize(size_t new_size) { values.resize(new_size); return *this; }
    this_t & Resize(size_t new_size, const VALUE_T & default_value) {
      values.resize(new_size, default_value);
      return *this;
    }
    void reserve(size_t size_cap) {
      static_assert(size_cap < MAX_SIZE || !IS_STATIC, "Static vector size too small for reserve.");
      if constexpr (!IS_STATIC) values.reserve(size_cap);
    }

    VALUE_T & back() { return values.back(); }
    const VALUE_T & back() const { return values.back(); }
    VALUE_T & front() { return values.front(); }
    const VALUE_T & front() const { return values.front(); }

    // Fill vector with a provided value.
    // 1 arg -> specify value only; fill entire vector.
    // 2 args -> also set starting point for fill; fill to end of vector.
    // 3 args -> also set count for fill.
    template <typename T>
    this_t & Fill(T && value, size_t start, size_t count) {
      while (--count > 0) values[start++] = value;
      values[start] = std::forward<T>(value);  // Move last (or only) value by move, if applicable.
      return *this;
    }

    // Allow fill to have optional arguments, with no "count" meaning to end of vector.
    template <typename T>
    this_t & Fill(T && value, size_t start=0) {
      Fill(std::forward<T>(value, start, size()-start));
      return *this;
    }

    template <typename T>
    this_t & Push(T && value, size_t count=1) {
      Resize(size()+count, std::forward<T>(value));
      return *this;
    }

    VALUE_T && Pop() {
      if constexpr (IS_STATIC) {
        return values.Pop();
      } else {
        auto out = values.back();
        values.pop_back();
        return out;
      }
    }

    template <typename T>
    void Insert(size_t pos, T && value, size_t count=1) {
      if constexpr (IS_STATIC) { values.Insert(pos, std::forward<T>(value), count); }
      else values.insert(values.begin()+pos, count, std::forward<T>(value));
    }

    template <typename T>
    void Erase(size_t pos, size_t count=1) {
      if constexpr (IS_STATIC) { values.Erase(pos, count); }
      else values.erase(values.begin()+pos, count);
    }

    // Remove [start_pos, end_pos) from this Vector and return it.
    this_t Copy(size_t start_pos, size_t count) {
      size_t end_pos = start_pos + count;
      emp_assert(start_pos <= size() && count <= size() && end_pos <= size());
      this_t out;
      if constexpr (MAX_SIZE) {
        std::copy(begin() + start_pos, begin() + end_pos, out.begin());
      }
      else { // Variable size vector
        out.reserve(count);
        std::copy(begin() + start_pos, begin() + end_pos, std::back_inserter(out));
      }
      return out;
    }

    // Remove [start_pos, end_pos) from this Vector and return it.
    this_t Extract(size_t start_pos, size_t count) {
      this_t out(Copy(start_pos, count));
      Erase(start_pos, count);
      return out;
    }
  };

}

#endif // #ifndef EMP_DATASTRUCT_VECTOR_HPP_INCLUDE
