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

  template < class Key, class T, class... Ts >
  class map : public std::map<Key, T, Ts...> {
  private:
    using this_t = emp::map<Key,T,Ts....>;
    using base_t = std::map<Key,T,Ts....>;

  public:
    using key_type = KEY;
    using mapped_type = T;
    using value_type = std::pair<const key_type,mapped_type>;
    using key_compare = base_t::key_compare;
    using value_compare = base_t::value_compare;
    using allocator_type = base_t::allocator_type;
    using reference = base_t::reference;
    using const_reference = base_t::const_reference;
    using pointer = base_t::pointer;
    using const_pointer = base_t::const_pointer;
    using iterator = base_t::iterator;
    using const_iterator = base_t::const_iterator;
    using reverse_iterator = base_t::reverse_iterator;
    using const_reverse_iterator = base_t::const_reverse_iterator;
    using difference_type = base_t::difference_type;
    using size_type = base_t::size_type;
  }

}

#endif



#endif
