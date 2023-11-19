/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2023.
 *
 *  @file ShortString.hpp
 *  @brief String substitute with a (default) max of 31 characters and always in place.
 *  @note Status: ALPHA
 * 
 *  A fixed number of chars are reserved for the string; errors will trigger for longer attempts.
 *  The last allocated byte stores the number of unused positions in the string; this naturally
 *  becomes zero (the final character in a string) when all positions are used.
 * 
 */

#ifndef EMP_TOOLS_SHORTSTRING_HPP_INCLUDE
#define EMP_TOOLS_SHORTSTRING_HPP_INCLUDE

#include <algorithm>
#include <string>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"

namespace emp {

  template <size_t NUM_CHARS, typename CHAR_T=char>
  class StaticString {
  private:
    static_assert(NUM_CHARS > 0, "StaticString must have at least one char available");
    static_assert(NUM_CHARS <= 65536, "StaticString size limited to 2 bytes (65536 chars).");
    static constexpr const size_t MAX_CHARS = NUM_CHARS - 1; // Save space for final '\0'

    emp::array<CHAR_T, NUM_CHARS> string;
    using ss_size_t = std::conditional_t<(NUM_CHARS <= 256), uint8_t, uint16_t>;
    ss_size_t str_size = 0;

    StaticString & CopyFrom(const CHAR_T * in, size_t len) {
      emp_assert(len <= MAX_CHARS, len, MAX_CHARS);
      resize(len);
      memcpy(string.data(), in, len);
      string[len] = '\0';
      return *this;
    }

    // Compare this string to another.
    // -1 means this is less, 0 means they are the same, +1 means this is greater.
    int Compare(const CHAR_T * in, size_t len) const {
      size_t min_len = std::min<size_t>(len, str_size);
      for (size_t i = 0; i < min_len; ++i) {
        if (string[i] != in[i]) {
          if (string[i] < in[i]) return -1;
          return 1;
        }
      }
      if (str_size != len) {
        if (str_size < len) return -1;
        return 1;
      }
      return 0;
    }
  public:
    StaticString() : str_size(0) { string[0] = '\0'; }
    StaticString(const StaticString &) = default;
    StaticString(const std::string & in) { CopyFrom(in.data(), in.size()); }
    StaticString(CHAR_T const * in) { CopyFrom(in, strlen(in)); }
    template <size_t SIZE> StaticString(CHAR_T in[SIZE]) { CopyFrom(in, SIZE-1); }

    StaticString & operator=(const StaticString &) = default;
    StaticString & operator=(const std::string & in) { return CopyFrom(in.data(), in.size()); }
    StaticString & operator=(CHAR_T const * in) { return CopyFrom(in, strlen(in)); }
    template <size_t SIZE> StaticString & operator=(CHAR_T in[SIZE]) { return CopyFrom(in, SIZE-1); }

    CHAR_T * data() { return string.data(); }
    const CHAR_T * data() const { return string.data(); }

    size_t size() const { return str_size; }

    void resize(size_t new_size) {
      emp_assert(new_size <= MAX_CHARS);
      str_size = new_size;
      string[new_size] = '\0';
    }

    CHAR_T & operator[](size_t id) {
      emp_assert(id < str_size);
      return string[id];
    }
    CHAR_T operator[](size_t id) const {
      emp_assert(id < str_size);
      return string[id];
    }

    // Comparisons

    template <typename T> bool operator==(const T & in) const { return Compare(in.data(), in.size()) == 0; }
    template <typename T> bool operator!=(const T & in) const { return Compare(in.data(), in.size()) != 0; }
    template <typename T> bool operator< (const T & in) const { return Compare(in.data(), in.size()) < 0; }
    template <typename T> bool operator<=(const T & in) const { return Compare(in.data(), in.size()) <= 0; }
    template <typename T> bool operator> (const T & in) const { return Compare(in.data(), in.size()) > 0; }
    template <typename T> bool operator>=(const T & in) const { return Compare(in.data(), in.size()) >= 0; }

    bool operator==(CHAR_T const * in) const { return Compare(in, strlen(in)) == 0; }
    bool operator!=(CHAR_T const * in) const { return Compare(in, strlen(in)) != 0; }
    bool operator< (CHAR_T const * in) const { return Compare(in, strlen(in)) < 0; }
    bool operator<=(CHAR_T const * in) const { return Compare(in, strlen(in)) <= 0; }
    bool operator> (CHAR_T const * in) const { return Compare(in, strlen(in)) > 0; }
    bool operator>=(CHAR_T const * in) const { return Compare(in, strlen(in)) >= 0; }

    // Type conversions
    operator CHAR_T *() { return string.data(); }
    operator const CHAR_T *() const { return string.data(); }
    operator std::string() const { return std::string(string.data()); }

    std::string AsString() const { return string.data(); }

    // Manipulations
    StaticString & push_back(CHAR_T c) {
      assert(str_size < MAX_CHARS);
      string[str_size++] = c;
      string[str_size] = '\0';
      return *this;
    }
  };

  using ShortString = emp::StaticString<31>;
}


#endif // #ifndef EMP_TOOLS_SHORTSTRING_HPP_INCLUDE
