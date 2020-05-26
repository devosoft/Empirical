/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file MapProxy.h
 *  @brief A proxy for indecies returned from any map type to ensure they are initialized.
 *  @note Status: ALPHA
 */

#ifndef EMP_MAP_PROXY_H
#define EMP_MAP_PROXY_H

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
    bool is_init;  ///< Has this value been set yet?
  public:
    MapProxy(T & in_value, bool in_init) : value(in_value), is_init(in_init) { }

    T & emp_GetValue() { emp_assert(is_init); return value; }
    const T & emp_GetValue() const { emp_assert(is_init); return value; }
    bool emp_IsInit() const { return is_init; }

    // A regular assignment is allowed with the creation of a new map value.
    template <typename R_T> T & operator=(R_T && _in) { is_init = true; return value = std::forward<R_T>(_in); }

    // Setup other assignment operators
    template <typename R_T> T & operator+=(R_T && _in) { return emp_GetValue() += std::forward<R_T>(_in); }
    template <typename R_T> T & operator-=(R_T && _in) { return emp_GetValue() -= std::forward<R_T>(_in); }
    template <typename R_T> T & operator*=(R_T && _in) { return emp_GetValue() *= std::forward<R_T>(_in); }
    template <typename R_T> T & operator/=(R_T && _in) { return emp_GetValue() /= std::forward<R_T>(_in); }
    template <typename R_T> T & operator%=(R_T && _in) { return emp_GetValue() %= std::forward<R_T>(_in); }
    template <typename R_T> T & operator&=(R_T && _in) { return emp_GetValue() &= std::forward<R_T>(_in); }
    template <typename R_T> T & operator|=(R_T && _in) { return emp_GetValue() |= std::forward<R_T>(_in); }
    template <typename R_T> T & operator^=(R_T && _in) { return emp_GetValue() ^= std::forward<R_T>(_in); }
    template <typename R_T> T & operator<<=(R_T && _in) { return emp_GetValue() <<= std::forward<R_T>(_in); }
    template <typename R_T> T & operator>>=(R_T && _in) { return emp_GetValue() >>= std::forward<R_T>(_in); }

    // Setup increment, decrement
    auto & operator++() { return ++emp_GetValue(); }
    auto & operator--() { return --emp_GetValue(); }
    auto operator++(int) { return emp_GetValue()++; }
    auto operator--(int) { return emp_GetValue()--; }

    // Setup basic arithmatic
    auto operator+() const { return +emp_GetValue(); }
    auto operator-() const { return -emp_GetValue(); }
    auto operator!() const { return !emp_GetValue(); }
    auto operator~() const { return ~emp_GetValue(); }
    template <typename R_T> auto operator + (const R_T & r) const { return emp_GetValue() + r; }
    template <typename R_T> auto operator - (const R_T & r) const { return emp_GetValue() - r; }
    template <typename R_T> auto operator * (const R_T & r) const { return emp_GetValue() * r; }
    template <typename R_T> auto operator / (const R_T & r) const { return emp_GetValue() / r; }
    template <typename R_T> auto operator % (const R_T & r) const { return emp_GetValue() % r; }

    template <typename R_T> auto operator & (const R_T & r) const { return emp_GetValue() & r; }
    template <typename R_T> auto operator | (const R_T & r) const { return emp_GetValue() | r; }
    template <typename R_T> auto operator ^ (const R_T & r) const { return emp_GetValue() ^ r; }
    template <typename R_T> auto operator << (const R_T & r) const { return emp_GetValue() << r; }
    template <typename R_T> auto operator >> (const R_T & r) const { return emp_GetValue() >> r; }
    template <typename R_T> auto operator && (const R_T & r) const { return emp_GetValue() && r; }
    template <typename R_T> auto operator || (const R_T & r) const { return emp_GetValue() || r; }

    // Setup comparison operators
    template <typename R_T> bool operator == (const R_T & r) const { return emp_GetValue() == r; }
    template <typename R_T> bool operator != (const R_T & r) const { return emp_GetValue() != r; }
    template <typename R_T> bool operator <  (const R_T & r) const { return emp_GetValue() < r; }
    template <typename R_T> bool operator <= (const R_T & r) const { return emp_GetValue() <= r; }
    template <typename R_T> bool operator >  (const R_T & r) const { return emp_GetValue() > r; }
    template <typename R_T> bool operator >= (const R_T & r) const { return emp_GetValue() >= r; }

    // Setup member access
    template <typename R_T> auto & operator [] (const R_T & r) { emp_assert(is_init); return value[r]; }
    template <typename R_T> const auto & operator [] (const R_T & r) const { emp_assert(is_init); return value[r]; }
    auto & operator * () { emp_assert(is_init); return *value; }
    const auto & operator * () const { emp_assert(is_init); return *value; }
    auto operator & () { emp_assert(is_init); return &value; }
    auto operator -> () { emp_assert(is_init); return value; }
    template <typename R_T> auto & operator ->* (const R_T & r) { emp_assert(is_init); return value->*r; }

    // Setup remaining misc operators.
    template <typename... R_Ts> auto operator () (R_Ts &&... rs) { emp_assert(is_init); return value( std::forward<R_Ts>(rs)... ); }
    template <typename... R_Ts> auto operator () (R_Ts &&... rs) const {
      emp_assert(is_init); return value( std::forward<R_Ts>(rs)... );
    }
    template <typename R_T> auto operator , (const R_T & r) { emp_assert(is_init); return value , r; }

    // Reflect over some common functionality for working with pointers.
    bool IsNull() const { return value.IsNull(); }
    void Delete() { value.Delete(); }
    void DeleteArray() { value.DeleteArray(); }

    // Dynamic casting to internal type.
    operator T&() { return value; } // Note that a non-const reference doesn't need var to be initialized yet.
    operator const T&() const { emp_assert(is_init); return value; }
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
