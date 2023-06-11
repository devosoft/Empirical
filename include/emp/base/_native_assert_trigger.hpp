/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file _native_assert_trigger.hpp
 *  @brief Native asssert trigger implementation.
 *  @note For internal use.
 */

#ifndef EMP_BASE__NATIVE_ASSERT_TRIGGER_HPP_INCLUDE
#define EMP_BASE__NATIVE_ASSERT_TRIGGER_HPP_INCLUDE

#include <iostream>
#include <sstream>
#include <string>

#include "_is_streamable.hpp"

namespace emp {

  constexpr bool assert_on = true;

  /// Base case for assert_print...
  inline void assert_print() { ; }

  /// Print out information about the next variable and recurse...
  template <typename T, typename... EXTRA>
  void assert_print(std::string name, T && val, EXTRA &&... extra) {
    if constexpr ( emp::is_streamable<decltype( std::cerr ), T>::value ) {
      // If we had a literal string fed in, print it as a message.
      if (name[0] == '"') {
        std::cerr << "MESSAGE: " << val << std::endl;
      }
      // Otherwise assume that we have a variable and print that.
      else {
        std::cerr << name << ": [" << val << "]" << std::endl;
      }
    } else std::cerr << name << ": (non-streamable type)" << std::endl;

    assert_print(std::forward<EXTRA>(extra)...);
  }

  template <typename... EXTRA>
  bool assert_trigger(std::string filename, size_t line, std::string expr, EXTRA &&... extra) {
    std::cerr << "Assert Error (In " << filename << " line " << line
              <<  "): " << expr << std::endl;
    assert_print(std::forward<EXTRA>(extra)...);

    return true; // do process subsequent abort
  }

} // namespace emp


#endif // #ifndef EMP_BASE__NATIVE_ASSERT_TRIGGER_HPP_INCLUDE
