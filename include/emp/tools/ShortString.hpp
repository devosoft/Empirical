/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file ShortString.hpp
 *  @brief String substitute with a max of 31 characters and always in place.
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

  template <size_t NUM_BYTES, typename CHAR_T=char>
  class StaticString {
  private:
    static_assert(NUM_BYTES <= 256, "Static String currently limited to 256 bytes (255 chars).");
    static constexpr const size_t MAX_CHARS = NUM_BYTES - 1;
    static constexpr const size_t SIZE_POS = NUM_BYTES - 1;  // Size info stored in the last byte.

    emp::array<CHAR_T, NUM_BYTES> string;

    CHAR_T & SizeByte() { return string[SIZE_POS]; }
    CHAR_T SizeByte() const { return string[SIZE_POS]; }

    StaticString & CopyFrom(const CHAR_T * in, size_t len) {
      resize(len);
      memcpy(string.data(), in, len);
      return *this;
    }

    // Compare this string to another.
    // -1 means this is less, 0 means they are the same, +1 means this is greater.
    int Compare(const CHAR_T * in, size_t len) const {
      size_t min_len = std::min(len, size());
      for (size_t i = 0; i < min_len; ++i) {
        if (string[i] != in[i]) {
          if (string[i] < in[i]) return -1;
          return 1;
        }
      }
      if (size() != len) {
        if (size() < len) return -1;
        return 1;
      }
      return 0;
    }
  public:
    StaticString() { string[0] = '\0'; SizeByte() = MAX_CHARS; }
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

    size_t size() const { return MAX_CHARS - SizeByte(); }

    void resize(size_t new_size) {
      emp_assert(new_size <= MAX_CHARS);
      SizeByte() = MAX_CHARS - new_size;
      string[new_size] = '\0';
    }

    CHAR_T & operator[](size_t id) {
      emp_assert(id < size());
      return string[id];
    }
    CHAR_T operator[](size_t id) const {
      emp_assert(id < size());
      return string[id];
    }


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

    operator CHAR_T *() { return string.data(); }
    operator const CHAR_T *() const { return string.data(); }
    operator std::string() const { return std::string(string.data()); }

    std::string AsString() const { return string.data(); }
  };

  using ShortString = emp::StaticString<32>;
}


#endif // #ifndef EMP_TOOLS_SHORTSTRING_HPP_INCLUDE
