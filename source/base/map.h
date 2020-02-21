/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file map.h
 *  @brief A drop-in wrapper for std::map and std:multimap; makes sure we create vars on access.
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

#include <initializer_list>
#include <map>

#include "assert.h"
#include "MapProxy.h"

// If debug is turned out, translate back to std::map
#ifdef EMP_NDEBUG

// Seemlessly translate emp::map to std::map
namespace emp {
  template <typename... Ts> using map = std::map<Ts...>;
}

// Seemlessly translate emp::multimap to std::multimap
namespace emp {
  template <typename... Ts> using multimap = std::multimap<Ts...>;
}

#else // #EMP_NDEBUG *not* set, so debug is ON.

namespace emp {

  template < class Key, class T, class... Ts >
  class map : public std::map<Key, T, Ts...> {
  private:
    using this_t = emp::map<Key,T,Ts...>;
    using base_t = std::map<Key,T,Ts...>;
    using proxy_t = MapProxy<std::decay_t<T>>;

  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const key_type,mapped_type>;
    using key_compare = typename base_t::key_compare;
    using value_compare = typename base_t::value_compare;
    using allocator_type = typename base_t::allocator_type;
    using reference = typename base_t::reference;
    using const_reference = typename base_t::const_reference;
    using pointer = typename base_t::pointer;
    using const_pointer = typename base_t::const_pointer;
    using iterator = typename base_t::iterator;
    using const_iterator = typename base_t::const_iterator;
    using reverse_iterator = typename base_t::reverse_iterator;
    using const_reverse_iterator = typename base_t::const_reverse_iterator;
    using difference_type = typename base_t::difference_type;
    using size_type = typename base_t::size_type;

    explicit map (const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(comp, alloc) { }
    explicit map (const allocator_type& alloc) : base_t(alloc) { }
    template <class InputIterator>
    map (InputIterator first, InputIterator last,
         const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(first, last, comp, alloc) { }
    map (const map& x) : base_t(x) { }
    map (const map& x, const allocator_type& alloc) : base_t(x, alloc) { }
    map (map && x) : base_t(std::move(x)) { }
    map (map && x, const allocator_type& alloc) : base_t(std::move(x), alloc) { }
    map (std::initializer_list<value_type> il, const key_compare& comp = key_compare(),
         const allocator_type& alloc = allocator_type())
      : base_t(il, comp, alloc) { }

    proxy_t operator[] (const Key & k) {
      const bool is_init = (this->find(k) != this->end());
      return proxy_t(base_t::operator[](k), is_init);
    }
  };

  template < class Key, class T, class... Ts >
  class multimap : public std::multimap<Key, T, Ts...> {
  private:
    using this_t = emp::multimap<Key,T,Ts...>;
    using base_t = std::multimap<Key,T,Ts...>;
    using proxy_t = MapProxy<std::decay_t<T>>;

  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<const key_type,mapped_type>;
    using key_compare = typename base_t::key_compare;
    using value_compare = typename base_t::value_compare;
    using allocator_type = typename base_t::allocator_type;
    using reference = typename base_t::reference;
    using const_reference = typename base_t::const_reference;
    using pointer = typename base_t::pointer;
    using const_pointer = typename base_t::const_pointer;
    using iterator = typename base_t::iterator;
    using const_iterator = typename base_t::const_iterator;
    using reverse_iterator = typename base_t::reverse_iterator;
    using const_reverse_iterator = typename base_t::const_reverse_iterator;
    using difference_type = typename base_t::difference_type;
    using size_type = typename base_t::size_type;

    explicit multimap (const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(comp, alloc) { }
    explicit multimap (const allocator_type& alloc) : base_t(alloc) { }
    template <class InputIterator>
    multimap (InputIterator first, InputIterator last,
         const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(first, last, comp, alloc) { }
    multimap (const multimap& x) : base_t(x) { }
    multimap (const multimap& x, const allocator_type& alloc) : base_t(x, alloc) { }
    multimap (multimap && x) : base_t(std::move(x)) { }
    multimap (multimap && x, const allocator_type& alloc) : base_t(std::move(x), alloc) { }
    multimap (std::initializer_list<value_type> il, const key_compare& comp = key_compare(),
         const allocator_type& alloc = allocator_type())
      : base_t(il, comp, alloc) { }

    proxy_t operator[] (const Key & k) {
      const bool is_init = (this->find(k) != this->end());
      return proxy_t(base_t::operator[](k), is_init);
    }
  };
}

#endif



#endif
