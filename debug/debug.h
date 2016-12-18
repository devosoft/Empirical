//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  Basic tools for use in developing high-assurance code.

#ifndef EMP_DEBUG_DEBUG
#define EMP_DEBUG_DEBUG

namespace emp {

  // The BlockRelease() function will halt compilation if NDEBUG is on.  It is useful to include
  // alongside debug print code that you want to remember to remove when you are done debugging.
#ifdef NDEBUG
  inline void BlockRelease() { static_assert(false); }
#else
  inline void BlockRelease() { ; }
#endif

  // The EMP_DEBUG macro executes its contents in debug mode, but otherwise ignores them.
#ifdef NDEBUG
#define EMP_DEBUG(...)
#else
#define EMP_DEBUG(...) __VA_ARGS__
#endif

};

#endif


