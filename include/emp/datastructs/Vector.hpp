/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2025-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/datastructs/Vector.hpp
 * @brief A scaled-up version of std::vector with additional functionality.
 * @note Status: ALPHA
 */

#pragma once

#ifndef INCLUDE_EMP_DATASTRUCTS_VECTOR_HPP_GUARD
#define INCLUDE_EMP_DATASTRUCTS_VECTOR_HPP_GUARD

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "../base/assert.hpp"
#include "../base/vector.hpp"
#include "../serialize/SerialPod.hpp"

#include "StaticVector.hpp"

namespace emp {

  /**
   * @brief A class that can be either a static or dynamic vector, with extra functionality.
   *
   * Vector wraps either emp::StaticVector (when MAX_SIZE > 0) or emp::vector (when MAX_SIZE == 0),
   * adding fluent, range-friendly manipulation and analysis helpers on top of the standard
   * std::vector-style interface.
   *
   * @tparam VALUE_T Value type that the vector will be using.
   * @tparam MAX_SIZE Maximum number of entries in vector (use 0 for unlimited)
   */
  template <typename VALUE_T, size_t MAX_SIZE = 0>
  class Vector {
  public:
    static constexpr bool IS_STATIC = static_cast<bool>(MAX_SIZE);

  private:
    using this_t    = Vector<VALUE_T, MAX_SIZE>;
    using static_t  = StaticVector<VALUE_T, MAX_SIZE>;
    using dynamic_t = emp::vector<VALUE_T>;
    using vec_t     = std::conditional_t<IS_STATIC, static_t, dynamic_t>;
    vec_t values;

  public:
    using value_type             = VALUE_T;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using pointer                = value_type *;
    using const_pointer          = const value_type *;
    using iterator               = decltype(std::declval<vec_t &>().begin());
    using const_iterator         = decltype(std::declval<const vec_t &>().begin());

    // === CONSTRUCTORS ===

    Vector()                           = default;
    Vector(const Vector &)             = default;
    Vector(Vector &&)                  = default;
    Vector & operator=(const Vector &) = default;
    Vector & operator=(Vector &&)      = default;
    ~Vector()                          = default;

    Vector(size_t count, const VALUE_T & value = VALUE_T()) : values(count, value) { }

    Vector(std::initializer_list<VALUE_T> init) : values(init) { }

    // Build from any input range of convertible values.
    template <typename IT>
    Vector(IT first, IT last) : values(first, last) { }

    Vector & operator=(std::initializer_list<VALUE_T> init) {
      values = vec_t(init);
      return *this;
    }

    void Serialize(SerialPod & pod) { pod(values); }

    // === COMPARISON ===

    [[nodiscard]] auto operator<=>(const this_t &) const = default;
    [[nodiscard]] bool operator==(const this_t &) const = default;


    // === ELEMENT ACCESS ===

    [[nodiscard]] VALUE_T & operator[](size_t pos) {
      emp_assert(pos < size(), pos, size());
      return values[pos];
    }

    [[nodiscard]] const VALUE_T & operator[](size_t pos) const {
      emp_assert(pos < size(), pos, size());
      return values[pos];
    }

    [[nodiscard]] VALUE_T & back() { emp_assert(size()); return values.back(); }
    [[nodiscard]] const VALUE_T & back() const { emp_assert(size()); return values.back(); }
    [[nodiscard]] VALUE_T & front() { emp_assert(size()); return values.front(); }
    [[nodiscard]] const VALUE_T & front() const { emp_assert(size()); return values.front(); }

    // === CAPACITY ===

    [[nodiscard]] size_t size() const { return values.size(); }
    [[nodiscard]] bool empty() const { return values.size() == 0; }
    [[nodiscard]] static constexpr size_t MaxSize() { return IS_STATIC ? MAX_SIZE : 0; }

    void reserve(size_t size_cap) {
      emp_assert(!IS_STATIC || size_cap <= MAX_SIZE, "Static vector size too small for reserve.",
                 size_cap, MAX_SIZE);
      if constexpr (!IS_STATIC) { values.reserve(size_cap); }
    }

    // === ITERATORS ===

    [[nodiscard]] auto begin() noexcept { return values.begin(); }
    [[nodiscard]] auto begin() const noexcept { return values.begin(); }
    [[nodiscard]] auto end() noexcept { return values.end(); }
    [[nodiscard]] auto end() const noexcept { return values.end(); }
    [[nodiscard]] auto cbegin() const noexcept { return values.begin(); }
    [[nodiscard]] auto cend() const noexcept { return values.end(); }
    [[nodiscard]] auto rbegin() noexcept { return std::reverse_iterator(end()); }
    [[nodiscard]] auto rbegin() const noexcept { return std::reverse_iterator(end()); }
    [[nodiscard]] auto rend() noexcept { return std::reverse_iterator(begin()); }
    [[nodiscard]] auto rend() const noexcept { return std::reverse_iterator(begin()); }

    // === SIZE MODIFIERS ===

    this_t & Resize(size_t new_size) {
      emp_assert(!IS_STATIC || new_size <= MAX_SIZE, new_size, MAX_SIZE);
      values.resize(new_size);
      return *this;
    }

    this_t & Resize(size_t new_size, const VALUE_T & default_value) {
      emp_assert(!IS_STATIC || new_size <= MAX_SIZE, new_size, MAX_SIZE);
      values.resize(new_size, default_value);
      return *this;
    }

    this_t & Clear() { Resize(0); return *this; }


    // === STACK-STYLE MODIFIERS ===

    // Append count copies of value to the end of the vector.
    template <typename T>
    this_t & Push(T && value, size_t count = 1) {
      const size_t old_size = size();
      Resize(old_size + count);
      // Fill the new region; move into the final slot when the source is an rvalue.
      for (size_t i = old_size; i + 1 < old_size + count; ++i) values[i] = value;
      values[old_size + count - 1] = std::forward<T>(value);
      return *this;
    }

    // Append an entire range to the end of the vector.
    template <typename RANGE_T>
    this_t & Append(const RANGE_T & range) {
      for (const auto & v : range) Push(v);
      return *this;
    }

    [[nodiscard]] VALUE_T Pop() {
      emp_assert(size(), "Cannot Pop() from an empty Vector.");
      if constexpr (IS_STATIC) {
        return values.Pop();
      } else {
        VALUE_T out = std::move(values.back());
        values.pop_back();
        return out;
      }
    }

    // === POSITIONAL MODIFIERS ===

    template <typename T>
    this_t & Insert(size_t pos, T && value, size_t count = 1) {
      emp_assert(pos <= size(), pos, size());
      emp_assert(!IS_STATIC || size() + count <= MAX_SIZE);
      if constexpr (IS_STATIC) {
        values.Insert(pos, std::forward<T>(value), count);
      } else {
        values.insert(values.begin() + pos, count, value);
      }
      return *this;
    }

    this_t & Erase(size_t pos, size_t count = 1) {
      emp_assert(pos + count <= size(), pos, count, size());
      if constexpr (IS_STATIC) {
        values.Erase(pos, count);
      } else {
        values.erase(values.begin() + pos, values.begin() + pos + count);
      }
      return *this;
    }

    // Fill vector with a provided value.
    // 1 arg  -> specify value only; fill entire vector.
    // 2 args -> also set starting point for fill; fill to end of vector.
    // 3 args -> also set count for fill.
    template <typename T>
    this_t & Fill(T && value, size_t start, size_t count) {
      emp_assert(start + count <= size(), start, count, size());
      if (count == 0) return *this;
      for (size_t i = 0; i + 1 < count; ++i) values[start++] = value;
      values[start] = std::forward<T>(value);  // Last (or only) slot: move if applicable.
      return *this;
    }

    template <typename T>
    this_t & Fill(T && value, size_t start = 0) {
      emp_assert(start <= size(), start, size());
      return Fill(std::forward<T>(value), start, size() - start);
    }

    // Copy [start_pos, start_pos+count) from this Vector and return it as a new Vector.
    [[nodiscard]] this_t Copy(size_t start_pos, size_t count) const {
      const size_t end_pos = start_pos + count;
      emp_assert(start_pos <= size() && end_pos <= size(), start_pos, count, size());
      this_t out;
      if constexpr (IS_STATIC) {
        out.Resize(count);
        std::copy(begin() + start_pos, begin() + end_pos, out.begin());
      } else {
        out.reserve(count);
        std::copy(begin() + start_pos, begin() + end_pos, std::back_inserter(out.values));
      }
      return out;
    }

    // Remove [start_pos, start_pos+count) from this Vector and return it as a new Vector.
    [[nodiscard]] this_t Extract(size_t start_pos, size_t count) {
      this_t out(Copy(start_pos, count));
      Erase(start_pos, count);
      return out;
    }

    this_t & Reverse() {
      std::reverse(values.begin(), values.end());
      return *this;
    }


    // === VECTOR ANALYSIS (const, non-mutating) ===

    // Add up all of the values in the vector. Returns value_type{} for an empty vector.
    [[nodiscard]] auto CalcSum() const {
      return std::accumulate(values.begin(), values.end(), value_type{});
    }

    // Multiply all values together. Returns value_type{1} for an empty vector.
    [[nodiscard]] auto CalcProduct() const {
      return std::accumulate(values.begin(), values.end(), value_type{1}, std::multiplies<>{});
    }

    // Arithmetic mean as a double. Requires a non-empty vector.
    [[nodiscard]] double CalcMean() const {
      emp_assert(size(), "CalcMean() requires a non-empty Vector.");
      return static_cast<double>(CalcSum()) / static_cast<double>(size());
    }

    [[nodiscard]] const VALUE_T & Min() const {
      emp_assert(size(), "Min() requires a non-empty Vector.");
      return *std::min_element(values.begin(), values.end());
    }

    [[nodiscard]] const VALUE_T & Max() const {
      emp_assert(size(), "Max() requires a non-empty Vector.");
      return *std::max_element(values.begin(), values.end());
    }

    // Index of the first element comparing equal to value, or size() if absent.
    [[nodiscard]] size_t FindIndex(const VALUE_T & value) const {
      auto it = std::find(values.begin(), values.end(), value);
      return static_cast<size_t>(std::distance(values.begin(), it));
    }

    [[nodiscard]] bool Has(const VALUE_T & value) const {
      return std::find(values.begin(), values.end(), value) != values.end();
    }

    // Count elements satisfying a predicate (or, with no predicate, equal to value).
    [[nodiscard]] size_t Count(const VALUE_T & value) const {
      return static_cast<size_t>(std::count(values.begin(), values.end(), value));
    }

    template <typename FUN_T>
    [[nodiscard]] size_t CountIf(FUN_T fun) const {
      return static_cast<size_t>(std::count_if(values.begin(), values.end(), fun));
    }

    [[nodiscard]] bool IsSorted() const {
      return std::is_sorted(values.begin(), values.end());
    }

    template <typename FUN_T>
    [[nodiscard]] bool Any(FUN_T fun) const {
      return std::any_of(values.begin(), values.end(), fun);
    }

    template <typename FUN_T>
    [[nodiscard]] bool All(FUN_T fun) const {
      return std::all_of(values.begin(), values.end(), fun);
    }


    // === VECTOR MANIPULATION (mutating, fluent) ===

    this_t & Sort() {
      std::sort(values.begin(), values.end());
      return *this;
    }

    template <typename COMPARE_T>
    this_t & Sort(COMPARE_T less_than_fun) {
      std::sort(values.begin(), values.end(), std::move(less_than_fun));
      return *this;
    }

    this_t & StableSort() {
      std::stable_sort(values.begin(), values.end());
      return *this;
    }

    template <typename COMPARE_T>
    this_t & StableSort(COMPARE_T less_than_fun) {
      std::stable_sort(values.begin(), values.end(), std::move(less_than_fun));
      return *this;
    }

    // Apply a function to each element in place. Function takes VALUE_T & (or by value).
    template <typename FUN_T>
    this_t & Apply(FUN_T fun) {
      for (auto & x : values) fun(x);
      return *this;
    }

    // Transform each element in place to fun(x).
    template <typename FUN_T>
    this_t & Transform(FUN_T fun) {
      std::transform(values.begin(), values.end(), values.begin(), std::move(fun));
      return *this;
    }

    // Remove every element for which fun(x) is true (erase-remove idiom).
    template <typename FUN_T>
    this_t & RemoveIf(FUN_T fun) {
      auto new_end = std::remove_if(values.begin(), values.end(), std::move(fun));
      const size_t new_size = static_cast<size_t>(std::distance(values.begin(), new_end));
      Resize(new_size);
      return *this;
    }

    // Keep only the elements for which fun(x) is true.
    template <typename FUN_T>
    this_t & KeepIf(FUN_T fun) {
      return RemoveIf([f = std::move(fun)](const VALUE_T & x) { return !f(x); });
    }

    // Remove consecutive duplicate elements (call Sort() first to remove all duplicates).
    this_t & MakeUnique() {
      auto new_end = std::unique(values.begin(), values.end());
      Resize(static_cast<size_t>(std::distance(values.begin(), new_end)));
      return *this;
    }


    // === ELEMENT-WISE ARITHMETIC ===

    // Add another vector element-by-element in place.
    this_t & AddEach(const this_t & other) {
      emp_assert(size() == other.size(), "Vectors must be the same size for AddEach.", size(), other.size());
      for (size_t i = 0; i < size(); ++i) values[i] += other[i];
      return *this;
    }

    // Multiply by another vector element-by-element in place.
    this_t & MulEach(const this_t & other) {
      emp_assert(size() == other.size(), "Vectors must be the same size for MulEach.", size(), other.size());
      for (size_t i = 0; i < size(); ++i) values[i] *= other[i];
      return *this;
    }

    // Subtract another vector element-by-element in place.
    this_t & SubEach(const this_t & other) {
      emp_assert(size() == other.size(), "Vectors must be the same size for SubEach.", size(), other.size());
      for (size_t i = 0; i < size(); ++i) values[i] -= other[i];
      return *this;
    }

    this_t & operator+=(const this_t & other) { return AddEach(other); }
    this_t & operator-=(const this_t & other) { return SubEach(other); }
    this_t & operator*=(const this_t & other) { return MulEach(other); }

    [[nodiscard]] this_t operator+(const this_t & other) const { return this_t(*this).AddEach(other); }
    [[nodiscard]] this_t operator-(const this_t & other) const { return this_t(*this).SubEach(other); }
    [[nodiscard]] this_t operator*(const this_t & other) const { return this_t(*this).MulEach(other); }


    // === STD::VECTOR COMPATIBILITY ===
    // Lowercase wrappers so emp::Vector is a drop-in replacement for std::vector.

    void resize(size_t n)                        { Resize(n); }
    void resize(size_t n, const VALUE_T & val)   { Resize(n, val); }
    void clear()                                 { Clear(); }

    void push_back(const VALUE_T & val)  { Push(val); }
    void push_back(VALUE_T && val)       { Push(std::move(val)); }
    void pop_back()                      { Pop(); }

    template <typename... ARGS>
    VALUE_T & emplace_back(ARGS &&... args) {
      if constexpr (IS_STATIC) {
        emp_assert(size() < MAX_SIZE, "emplace_back: StaticVector is full.", size(), MAX_SIZE);
        const size_t idx = size();
        values.resize(idx + 1);
        values[idx] = VALUE_T(std::forward<ARGS>(args)...);
        return values[idx];
      } else {
        return values.emplace_back(std::forward<ARGS>(args)...);
      }
    }

    [[nodiscard]] VALUE_T &       at(size_t pos)       { emp_assert(pos < size(), pos, size()); return values[pos]; }
    [[nodiscard]] const VALUE_T & at(size_t pos) const { emp_assert(pos < size(), pos, size()); return values[pos]; }

    // For static vectors the backing array is always allocated; &values[0] is always valid.
    [[nodiscard]] VALUE_T *       data() noexcept       {
      if constexpr (IS_STATIC) return &values[0]; else return values.data();
    }
    [[nodiscard]] const VALUE_T * data() const noexcept {
      if constexpr (IS_STATIC) return &values[0]; else return values.data();
    }

    [[nodiscard]] size_t capacity() const noexcept {
      if constexpr (IS_STATIC) return MAX_SIZE; else return values.capacity();
    }

    [[nodiscard]] size_t max_size() const noexcept {
      if constexpr (IS_STATIC) return MAX_SIZE; else return values.max_size();
    }

    void swap(this_t & other) noexcept { std::swap(values, other.values); }

    void shrink_to_fit() { if constexpr (!IS_STATIC) values.shrink_to_fit(); }

    void assign(size_t count, const VALUE_T & val) { Resize(count); Fill(val); }

    template <typename IT>
    void assign(IT first, IT last) { Clear(); for (; first != last; ++first) push_back(*first); }

    void assign(std::initializer_list<VALUE_T> init) { *this = init; }

    // Iterator-based insert: convert iterator to index, delegate to Insert().
    iterator insert(const_iterator pos, const VALUE_T & val) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      Insert(idx, val);
      return begin() + idx;
    }

    iterator insert(const_iterator pos, VALUE_T && val) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      Insert(idx, std::move(val));
      return begin() + idx;
    }

    iterator insert(const_iterator pos, size_t count, const VALUE_T & val) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      Insert(idx, val, count);
      return begin() + idx;
    }

    template <typename IT>
    iterator insert(const_iterator pos, IT first, IT last) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      size_t cur = idx;
      for (; first != last; ++first, ++cur) Insert(cur, *first);
      return begin() + idx;
    }

    iterator insert(const_iterator pos, std::initializer_list<VALUE_T> init) {
      return insert(pos, init.begin(), init.end());
    }

    template <typename... ARGS>
    iterator emplace(const_iterator pos, ARGS &&... args) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      Insert(idx, VALUE_T(std::forward<ARGS>(args)...));
      return begin() + idx;
    }

    // Iterator-based erase: convert iterator to index, delegate to Erase().
    iterator erase(const_iterator pos) {
      const size_t idx = static_cast<size_t>(std::distance(cbegin(), pos));
      Erase(idx);
      return begin() + idx;
    }

    iterator erase(const_iterator first, const_iterator last) {
      const size_t idx   = static_cast<size_t>(std::distance(cbegin(), first));
      const size_t count = static_cast<size_t>(std::distance(first, last));
      Erase(idx, count);
      return begin() + idx;
    }
  };

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_DATASTRUCTS_VECTOR_HPP_GUARD