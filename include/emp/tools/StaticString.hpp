/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2022-2026 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/tools/StaticString.hpp
 * @brief String substitute with fixed max character count; always stored in-place.
 * @note Status: ALPHA
 *
 * Stores up to MAX_CHARS = NUM_CHARS - 1 characters w/ final slot reserved for '\0'.
 * Current length is tracked in ss_size_t field (uint8_t for <256, uint16_t otherwise).
 *
 * All operations are constexpr can be evaluated at compile time.  Class is a structural type
 * (all data members are public) and can therefore be used as a non-type template parameter.
 */

#pragma once

#ifndef INCLUDE_EMP_TOOLS_STATIC_STRING_HPP_GUARD
#define INCLUDE_EMP_TOOLS_STATIC_STRING_HPP_GUARD

#include <algorithm>
#include <compare>
#include <string>
#include <string_view>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"

namespace emp {

  template <size_t NUM_CHARS, typename CHAR_T = char>
  struct StaticString {
    static_assert(NUM_CHARS > 0,      "StaticString must have at least one char available");
    static_assert(NUM_CHARS <= 65536, "StaticString size limited to 2 bytes (65536 chars).");
    static constexpr size_t MAX_CHARS = NUM_CHARS - 1;  // One slot reserved for '\0'

    using ss_size_t = std::conditional_t<(NUM_CHARS <= 256), uint8_t, uint16_t>;

    // Public data members — required for use as a non-type template parameter (C++20).
    emp::array<CHAR_T, NUM_CHARS> string = {};  // zero-init so all elements are always initialized
    ss_size_t str_size = 0;

  private:
    constexpr StaticString & CopyFrom(const CHAR_T * in, size_t len) {
      emp_assert(len <= MAX_CHARS, len, MAX_CHARS);
      resize(len);
      std::copy(in, in + len, string.data());
      string[len] = '\0';
      return *this;
    }

    [[nodiscard]] constexpr std::string_view AsView() const noexcept {
      return {string.data(), str_size};
    }

  public:
    constexpr StaticString() = default;

    constexpr StaticString(const StaticString &) = default;

    constexpr StaticString(const std::string & in) { CopyFrom(in.data(), in.size()); }
    constexpr StaticString(CHAR_T const * in) {
      CopyFrom(in, std::char_traits<CHAR_T>::length(in));
    }

    // Reference-to-array form: SIZE is deduced from the actual array, not a decayed pointer.
    template <size_t SIZE>
    constexpr StaticString(const CHAR_T (&in)[SIZE]) { CopyFrom(in, SIZE - 1); }

    constexpr StaticString & operator=(const StaticString &) = default;

    constexpr StaticString & operator=(const std::string & in) {
      return CopyFrom(in.data(), in.size());
    }
    constexpr StaticString & operator=(CHAR_T const * in) {
      return CopyFrom(in, std::char_traits<CHAR_T>::length(in));
    }

    template <size_t SIZE>
    constexpr StaticString & operator=(const CHAR_T (&in)[SIZE]) {
      return CopyFrom(in, SIZE - 1);
    }

    // --- Accessors ---

    [[nodiscard]] constexpr CHAR_T *       data()    noexcept { return string.data(); }
    [[nodiscard]] constexpr const CHAR_T * data()    const noexcept { return string.data(); }
    [[nodiscard]] constexpr const CHAR_T * c_str()   const noexcept { return string.data(); }

    [[nodiscard]] constexpr size_t size()     const noexcept { return str_size; }
    [[nodiscard]] constexpr size_t max_size() const noexcept { return MAX_CHARS; }
    [[nodiscard]] constexpr bool   empty()    const noexcept { return str_size == 0; }

    constexpr CHAR_T *       begin()       noexcept { return string.data(); }
    constexpr CHAR_T *       end()         noexcept { return string.data() + str_size; }
    constexpr const CHAR_T * begin() const noexcept { return string.data(); }
    constexpr const CHAR_T * end()   const noexcept { return string.data() + str_size; }

    constexpr void resize(size_t new_size) {
      emp_assert(new_size <= MAX_CHARS);
      str_size         = static_cast<ss_size_t>(new_size);
      string[new_size] = '\0';
    }

    constexpr void resize(size_t new_size, CHAR_T filler) {
      emp_assert(new_size <= MAX_CHARS);
      for (size_t i = str_size; i < new_size; ++i) { string[i] = filler; }
      resize(new_size);
    }

    [[nodiscard]] constexpr CHAR_T & operator[](size_t id) {
      emp_assert(id < str_size);
      return string[id];
    }

    [[nodiscard]] constexpr CHAR_T operator[](size_t id) const {
      emp_assert(id < str_size);
      return string[id];
    }

    // --- Comparisons ---
    // Each type pair provides operator== (generates !=) and operator<=> (generates <, <=, >, >=).

    template <size_t NUM_CHARS2>
    [[nodiscard]] constexpr bool operator==(const StaticString<NUM_CHARS2, CHAR_T> & rhs) const noexcept {
      return AsView() == std::string_view{rhs.data(), rhs.size()};
    }
    template <size_t NUM_CHARS2>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(
        const StaticString<NUM_CHARS2, CHAR_T> & rhs) const noexcept {
      return AsView() <=> std::string_view{rhs.data(), rhs.size()};
    }

    [[nodiscard]] constexpr bool operator==(const std::string & rhs) const noexcept {
      return AsView() == std::string_view{rhs};
    }
    [[nodiscard]] constexpr std::strong_ordering operator<=>(const std::string & rhs) const noexcept {
      return AsView() <=> std::string_view{rhs};
    }

    [[nodiscard]] constexpr bool operator==(const CHAR_T * rhs) const noexcept {
      return AsView() == std::string_view{rhs};
    }
    [[nodiscard]] constexpr std::strong_ordering operator<=>(const CHAR_T * rhs) const noexcept {
      return AsView() <=> std::string_view{rhs};
    }

    // --- Type conversions ---

    [[nodiscard]] constexpr operator CHAR_T *() { return string.data(); }
    [[nodiscard]] constexpr operator const CHAR_T *() const noexcept { return string.data(); }
    [[nodiscard]] constexpr operator std::string() const { return {string.data(), str_size}; }
    [[nodiscard]] constexpr operator std::string_view() const noexcept { return AsView(); }

    [[nodiscard]] constexpr std::string AsString() const { return {string.data(), str_size}; }
    [[nodiscard]] constexpr std::string_view AsStringView() const noexcept { return AsView(); }

    // --- Manipulations ---

    constexpr StaticString & push_back(CHAR_T c) {
      emp_assert(str_size < MAX_CHARS);
      string[str_size++] = c;
      string[str_size]   = '\0';
      return *this;
    }

    constexpr StaticString & append(const CHAR_T * in_str, size_t len) {
      emp_assert(str_size + len <= MAX_CHARS);
      std::copy(in_str, in_str + len, string.data() + str_size);
      str_size         = static_cast<ss_size_t>(str_size + len);
      string[str_size] = '\0';
      return *this;
    }

    constexpr StaticString & append(const std::string & in_str) {
      return append(in_str.data(), in_str.size());
    }

    template <size_t SIZE>
    constexpr StaticString & append(const CHAR_T (&in_str)[SIZE]) {
      return append(in_str, SIZE - 1);
    }

    template <size_t NUM_CHARS2>
    constexpr StaticString & append(const StaticString<NUM_CHARS2, CHAR_T> & in_str) {
      return append(in_str.data(), in_str.size());
    }
  };

  using ShortString = emp::StaticString<31>;

  // Deduction guide: StaticString("hello") deduces StaticString<6> (includes null terminator).
  template <size_t N, typename CHAR_T>
  StaticString(const CHAR_T (&)[N]) -> StaticString<N, CHAR_T>;

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_TOOLS_STATIC_STRING_HPP_GUARD
