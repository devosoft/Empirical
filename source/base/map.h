/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file map.h
 *  @brief A drop-in wrapper for std::map; makes sure we create vars on access.
 *  @note Status: ALPHA
 *
 *  This class is a drop-in wrapper for std::map, adding on debbing where the indexing operator
 *  [square brackets] returns a proxy.  That proxy can either be assigned to OR convert an
 *  existing value ONLY if it exists.  This mechanism ensures that we don't accidentally write
 *  a default value to a map when all we meant to do was read from it, but had a typo.
 *  If EMP_NDEBUG is set then it reverts back to std::map.
 */

#ifndef EMP_MAP_H
#define EMP_MAP_H

// If debug is turned out, translate back to std::map
#ifdef EMP_NDEBUG

// Seemlessly translate emp::map to std::map
namespace emp {
  template <typename... Ts> using map = std::map<Ts...>;
}

#else // #EMP_NDEBUG *not* set, so debug is ON.

namespace emp {


}

#endif



#endif
