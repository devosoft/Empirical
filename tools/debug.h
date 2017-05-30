//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Basic tools for use in developing high-assurance code.
//  Status: BETA

#ifndef EMP_DEBUG_DEBUG
#define EMP_DEBUG_DEBUG

#include <set>
#include <string>
#include <type_traits>

namespace emp {

  // The BlockRelease() function will halt compilation if NDEBUG is on.  It is useful to include
  // alongside debug print code that you want to remember to remove when you are done debugging.
#ifdef NDEBUG
  #define BlockRelease(BLOCK) static_assert(!BLOCK, "Release blocked due to debug material.")
#else
  #define BlockRelease(BLOCK)
#endif

  // The EMP_DEBUG macro executes its contents in debug mode, but otherwise ignores them.
#ifdef NDEBUG
#define EMP_DEBUG(...)
#else
#define EMP_DEBUG(...) __VA_ARGS__
#endif

  // The Depricated() functon prints its contents exactly once.
  static void Depricated(const std::string & name, const std::string & desc="") {
    static std::set<std::string> name_set;
    if (name_set.count(name) == 0) {
      std::cerr << "Deprication WARNING: " << name << std::endl;
      if (desc != "") std::cerr << desc << std::endl;
      name_set.insert(name);
    }
  }

};

#endif
