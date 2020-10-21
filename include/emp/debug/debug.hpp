/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2015-2017
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

  /// BlockRelease() will halt compilation if NDEBUG is on.  It is useful to include alongside
  /// debug print code that you want to remember to remove when you are done debugging.
#ifdef NDEBUG
  #define BlockRelease(BLOCK) static_assert(!BLOCK, "Release blocked due to debug material.")
#else
  #define BlockRelease(BLOCK)
#endif

  /// The EMP_DEBUG macro executes its contents in debug mode, but otherwise ignores them.
#ifdef NDEBUG
#define EMP_DEBUG(...)
#else
#define EMP_DEBUG(...) __VA_ARGS__
#endif

  /// Depricated() prints its contents exactly once to notify a user of a depricated function.
  static void Depricated(const std::string & name, const std::string & desc="") {
    static std::set<std::string> name_set;
    if (name_set.count(name) == 0) {
      std::cerr << "Deprication WARNING: " << name << std::endl;
      if (desc != "") std::cerr << desc << std::endl;
      name_set.insert(name);
    }
  }

}

#endif
