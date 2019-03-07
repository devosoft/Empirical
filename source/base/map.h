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

#include <initializer_list>
#include <map>

#include "assert.h"

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
    using this_t = emp::map<Key,T,Ts...>;
    using base_t = std::map<Key,T,Ts...>;

    // The Proxy class is returned in the place of a mapped type to track usage.
    class Proxy {
    private:
      T & value;
    public:
      Proxy(T & in_value) : value(in_value) { }

      template <typename ASSIGN_T>
      T & operator=(ASSIGN_T && _in) { return value = std::forward<ASSIGN_T>(_in); }

      // Make sure basic match can be done with proxies.
      template <typename R_T> auto operator + (const R_T & r) { return value + r; }
      template <typename R_T> auto operator - (const R_T & r) { return value - r; }
      template <typename R_T> auto operator * (const R_T & r) { return value * r; }
      template <typename R_T> auto operator / (const R_T & r) { return value / r; }
      template <typename R_T> auto operator % (const R_T & r) { return value % r; }

      operator T&() { return value; }
      T* operator &() { return &value; }

    };

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

    Proxy operator[] (const Key & k) { return Proxy(base_t::operator[](k)); };
    Proxy operator[] (Key && k) { return Proxy(base_t::operator[]( std::forward<Key>(k) )); };
  };

}

#endif



#endif
