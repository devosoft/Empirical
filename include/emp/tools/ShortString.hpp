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

#ifndef EMP_TOOLS_SHORT_STRING_HPP_INCLUDE
#define EMP_TOOLS_SHORT_STRING_HPP_INCLUDE

#include <string>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/notify.hpp"

namespace emp {

  class ShortString {
  private:
    static constexpr const size_t NUM_BYTES = 32;
    static constexpr const size_t MAX_CHARS = NUM_BYTES - 1;
    static constexpr const size_t SIZE_POS = NUM_BYTES - 1;

    emp::array<char, NUM_BYTES> string;

    char & SizeByte() { return string[SIZE_POS]; }
    char SizeByte() const { return string[SIZE_POS]; }
  public:
    ShortString() { string[0] = '\0'; SizeByte() = MAX_CHARS; }
    ShortString(const ShortString &) = default;

    ShortString & operator=(const ShortString &) = default;
    ShortString & operator=(const std::string & in) {
      resize(in.size());
      memcpy(string.data(), in.data(), in.size());
    }
    ShortString & operator=(char * in) {
      resize(strlen(in));
      memcpy(string.data(), in, size());
    }

    size_t size() const { return MAX_CHARS - SizeByte(); }

    void resize(size_t new_size) {
      emp_assert(new_size <= MAX_CHARS);
      SizeByte() = MAX_CHARS - new_size;
      string[new_size] = '\0';
    }

    char & operator[](size_t id) {
      emp_assert(id < size());
      return string[id];
    }
    char operator[](size_t id) const {
      emp_assert(id < size());
      return string[id];
    }

    operator char *() { return string.data(); }
  };

}

#endif