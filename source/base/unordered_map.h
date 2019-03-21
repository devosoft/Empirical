/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file unordered_map.h
 *  @brief A drop-in wrapper for std::unordered_map and unordered_multi_map; makes sure we create vars on access.
 *  @note Status: ALPHA
 */

#ifndef EMP_UNORDERED_MAP_H
#define EMP_UNORDERED_MAP_H

#include <initializer_list>
#include <unordered_map>

#include "assert.h"
#include "MapProxy.h"

// If debug is turned out, translate back to std::map
#ifdef EMP_NDEBUG

// Seemlessly translate emp::unordered_map to std::unordered_map
namespace emp {
  template <typename... Ts> using unordered_map = std::unordered_map<Ts...>;
}

// Seemlessly translate emp::unordered_multimap to std::unordered_multimap
namespace emp {
  template <typename... Ts> using unordered_multimap = std::unordered_multimap<Ts...>;
}

#else // #EMP_NDEBUG *not* set, so debug is ON.

namespace emp {

  template < class Key, class T, class... Ts >
  class unordered_map : public std::unordered_map<Key, T, Ts...> {
  private:
    using this_t = emp::unordered_map<Key,T,Ts...>;
    using base_t = std::unordered_map<Key,T,Ts...>;
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

    explicit unordered_map (const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(comp, alloc) { }
    explicit unordered_map (const allocator_type& alloc) : base_t(alloc) { }
    template <class InputIterator>
    unordered_map (InputIterator first, InputIterator last,
         const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(first, last, comp, alloc) { }
    unordered_map (const unordered_map& x) : base_t(x) { }
    unordered_map (const unordered_map& x, const allocator_type& alloc) : base_t(x, alloc) { }
    unordered_map (unordered_map && x) : base_t(std::move(x)) { }
    unordered_map (unordered_map && x, const allocator_type& alloc) : base_t(std::move(x), alloc) { }
    map (std::initializer_list<value_type> il, const key_compare& comp = key_compare(),
         const allocator_type& alloc = allocator_type())
      : base_t(il, comp, alloc) { }

    proxy_t operator[] (const Key & k) {
      const bool is_init = (this->find(k) != this->end());
      return proxy_t(base_t::operator[](k), is_init);
    }
  };

  template < class Key, class T, class... Ts >
  class unordered_multimap : public std::unordered_multimap<Key, T, Ts...> {
  private:
    using this_t = emp::unordered_multimap<Key,T,Ts...>;
    using base_t = std::unordered_multimap<Key,T,Ts...>;
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

    explicit unordered_multimap (const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(comp, alloc) { }
    explicit unordered_multimap (const allocator_type& alloc) : base_t(alloc) { }
    template <class InputIterator>
    unordered_multimap (InputIterator first, InputIterator last,
         const key_compare& comp = key_compare(), const allocator_type& alloc = allocator_type())
      : base_t(first, last, comp, alloc) { }
    unordered_multimap (const unordered_multimap& x) : base_t(x) { }
    unordered_multimap (const unordered_multimap& x, const allocator_type& alloc) : base_t(x, alloc) { }
    unordered_multimap (unordered_multimap && x) : base_t(std::move(x)) { }
    unordered_multimap (unordered_multimap && x, const allocator_type& alloc) : base_t(std::move(x), alloc) { }
    unordered_multimap (std::initializer_list<value_type> il, const key_compare& comp = key_compare(),
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
