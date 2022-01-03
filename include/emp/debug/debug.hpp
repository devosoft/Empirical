/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2022
 *
 *  @file  debug.hpp
 *  @brief Basic tools for use in developing high-assurance code.
 *  @note Status: BETA
 */

#ifndef EMP_DEBUG_H
#define EMP_DEBUG_H

#include <iostream>
#include <set>
#include <string>
#include <type_traits>

#define EMP_DEBUG_PRINT(...) std::cout << "[" << #__VA_ARGS__ << "] = " << __VA_ARGS__ << std::endl

namespace emp {

  /// BlockRelease(true) will halt compilation if NDEBUG is on and EMP_NO_BLOCK is off.
  /// It is useful to include alongside debug code that you want to remember to remove when you
  /// are done debugging; it is automatically included with the emp_debug() function below.
  /// If you want to intentionally compile in release mode, make sure to define EMP_NO_BLOCK.
#ifdef NDEBUG
  #ifdef EMP_NO_BLOCK
    #define BlockRelease(BLOCK)
  #else
    #define BlockRelease(BLOCK) \\
      std::cerr << "Release block at " << __FILE___ << ", line " << __LINE__ << std::endl;\\
      static_assert(!BLOCK, "Release blocked due to debug material.")
  #endif
#else
  #define BlockRelease(BLOCK)
#endif

  /// The EMP_DEBUG macro executes its contents in debug mode, but otherwise ignores them.
  /// test_debug() can be used inside of an if-constexpr for code you want only in debug mode.
#ifdef NDEBUG
  #define EMP_DEBUG(...)
  constexpr bool test_debug() { return false; }
#else
  #define EMP_DEBUG(...) __VA_ARGS__
  constexpr bool test_debug() { return true; }
#endif

  template<typename... Ts>
  void emp_debug_print(Ts &&... args) {
    (std::cerr <<  ... << std::forward<Ts>(args));
    std::cerr << std::endl;
  }

  /// emp_debug() will print its contents as a message in debug mode and BLOCK release mode until
  /// it is removed.  It's a useful too for printing "Ping1", "Ping2", etc, but no forgetting to
  /// remove them.
  #define emp_debug(...) { BlockRelease(true); emp::emp_debug_print(__VA_ARGS__); }
}

#endif
