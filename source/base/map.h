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

  // The MapProxy class is returned in the place of a mapped type to track usage.
  // @CAO: Note that ideally we should have two proxy classes; one for when the wrapped type is a
  //       class itself and we can derive from it (thus, perhaps, keeping member functions working?)
  //       and the other being when it is a base type (and there are no member functions).
  //       Fundamentally this problem comes from the fact that we cannot overload the . operator,
  //       and thus cannot get it working directly for the Proxy class.
  template <typename T>
  class MapProxy {
  private:
    T & value;
  public:
    MapProxy(T & in_value) : value(in_value) { }

    T & emp_GetValue() { return value; }
    const T & emp_GetValue() const { return value; }

    // Setup assignment operators
    template <typename R_T> T & operator=(R_T && _in) { return value = std::forward<R_T>(_in); }
    template <typename R_T> T & operator+=(R_T && _in) { return value += std::forward<R_T>(_in); }
    template <typename R_T> T & operator-=(R_T && _in) { return value -= std::forward<R_T>(_in); }
    template <typename R_T> T & operator*=(R_T && _in) { return value *= std::forward<R_T>(_in); }
    template <typename R_T> T & operator/=(R_T && _in) { return value /= std::forward<R_T>(_in); }
    template <typename R_T> T & operator%=(R_T && _in) { return value %= std::forward<R_T>(_in); }
    template <typename R_T> T & operator&=(R_T && _in) { return value &= std::forward<R_T>(_in); }
    template <typename R_T> T & operator|=(R_T && _in) { return value |= std::forward<R_T>(_in); }
    template <typename R_T> T & operator^=(R_T && _in) { return value ^= std::forward<R_T>(_in); }
    template <typename R_T> T & operator<<=(R_T && _in) { return value <<= std::forward<R_T>(_in); }
    template <typename R_T> T & operator>>=(R_T && _in) { return value >>= std::forward<R_T>(_in); }

    // Setup increment, decrement
    auto & operator++() { return ++value; }
    auto & operator--() { return --value; }
    auto operator++(int) { return value++; }
    auto operator--(int) { return value--; }

    // Setup basic arithmatic
    auto operator+() { return +value; }
    auto operator-() { return -value; }
    auto operator!() { return !value; }
    auto operator~() { return ~value; }
    template <typename R_T> auto operator + (const R_T & r) { return value + r; }
    template <typename R_T> auto operator - (const R_T & r) { return value - r; }
    template <typename R_T> auto operator * (const R_T & r) { return value * r; }
    template <typename R_T> auto operator / (const R_T & r) { return value / r; }
    template <typename R_T> auto operator % (const R_T & r) { return value % r; }

    template <typename R_T> auto operator & (const R_T & r) { return value & r; }
    template <typename R_T> auto operator | (const R_T & r) { return value | r; }
    template <typename R_T> auto operator ^ (const R_T & r) { return value ^ r; }
    template <typename R_T> auto operator << (const R_T & r) { return value << r; }
    template <typename R_T> auto operator >> (const R_T & r) { return value >> r; }
    template <typename R_T> auto operator && (const R_T & r) { return value && r; }
    template <typename R_T> auto operator || (const R_T & r) { return value || r; }

    // Setup comparison operators
    template <typename R_T> bool operator == (const R_T & r) { return value == r; }
    template <typename R_T> bool operator != (const R_T & r) { return value != r; }
    template <typename R_T> bool operator <  (const R_T & r) { return value < r; }
    template <typename R_T> bool operator <= (const R_T & r) { return value <= r; }
    template <typename R_T> bool operator >  (const R_T & r) { return value > r; }
    template <typename R_T> bool operator >= (const R_T & r) { return value >= r; }

    // Setup member access
    template <typename R_T> auto & operator [] (const R_T & r) { return value[r]; }
    auto & operator * () { return *value; }
    auto operator & () { return &value; }
    auto operator -> () { return value; }
    template <typename R_T> auto & operator ->* (const R_T & r) { return value->*r; }

    // Setup remaining misc operators.
    template <typename... R_Ts> auto operator () (R_Ts &&... rs) { return value( std::forward<R_Ts>(rs)... ); }
    template <typename R_T> auto operator , (const R_T & r) { return value , r; }

    // Dynamic casting to internal type.
    operator T&() { return value; }
  };

  /// A type trait to determine if a class is a MapProxy
  template <typename> struct is_MapProxy : public std::false_type { };

  template <typename T>
  struct is_MapProxy<MapProxy<T>> : public std::true_type { };

  // Build externaly binary operators with MapProxy as the second argument.
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator + (T1 v1, const MapProxy<T2> & v2) { return v1 + v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator - (T1 v1, const MapProxy<T2> & v2) { return v1 - v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator * (T1 v1, const MapProxy<T2> & v2) { return v1 * v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator / (T1 v1, const MapProxy<T2> & v2) { return v1 / v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator % (T1 v1, const MapProxy<T2> & v2) { return v1 % v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator & (T1 v1, const MapProxy<T2> & v2) { return v1 & v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator | (T1 v1, const MapProxy<T2> & v2) { return v1 | v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator ^ (T1 v1, const MapProxy<T2> & v2) { return v1 ^ v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator << (T1 v1, const MapProxy<T2> & v2) { return v1 << v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator >> (T1 v1, const MapProxy<T2> & v2) { return v1 >> v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator && (T1 v1, const MapProxy<T2> & v2) { return v1 && v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator || (T1 v1, const MapProxy<T2> & v2) { return v1 || v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator == (T1 v1, const MapProxy<T2> & v2) { return v1 == v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator != (T1 v1, const MapProxy<T2> & v2) { return v1 != v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator <  (T1 v1, const MapProxy<T2> & v2) { return v1 < v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator <= (T1 v1, const MapProxy<T2> & v2) { return v1 <= v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator >  (T1 v1, const MapProxy<T2> & v2) { return v1 > v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator >= (T1 v1, const MapProxy<T2> & v2) { return v1 >= v2.emp_GetValue(); }
  template <typename T1, typename T2, typename std::enable_if<is_MapProxy<T1>() == false>::type* = nullptr>
  auto operator , (T1 v1, const MapProxy<T2> & v2) { return v1 , v2.emp_GetValue(); }

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

    proxy_t operator[] (const Key & k) { return proxy_t(base_t::operator[](k)); };
    proxy_t operator[] (Key && k) { return proxy_t(base_t::operator[]( std::forward<Key>(k) )); };
  };

}

// A crude, generic printing function for emp::MapProxy.
template <typename T>
std::ostream & operator<<(std::ostream & out, const typename emp::MapProxy<T> & p) {
  out << p.emp_GetValue();
  return out;
}

template <typename T>
std::istream & operator>>(std::istream & is, typename emp::MapProxy<T> & p) {
  is >> p.emp_GetValue();
  return is;
}

#endif



#endif
