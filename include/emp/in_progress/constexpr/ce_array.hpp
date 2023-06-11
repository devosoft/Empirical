/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016
 *
 *  @file ce_array.hpp
 *  @brief ce_array defines a limited array object for use within a constexpr class or function.
 *
 * STATUS: ALPHA
 *
 */

#ifndef EMP_IN_PROGRESS_CONSTEXPR_CE_ARRAY_HPP_INCLUDE
#define EMP_IN_PROGRESS_CONSTEXPR_CE_ARRAY_HPP_INCLUDE

#include <type_traits>

#include "../base/assert.hpp"

namespace emp {

  template <typename T, size_t N>
  class ce_array {
  private:
    static constexpr size_t p1_size = N/2;
    static constexpr size_t p2_size = N - p1_size;
    ce_array<T, p1_size> p1;   // First half of values.
    ce_array<T, p2_size> p2;   // Second half of values.

  public:
    using value_type = T;

    constexpr ce_array(const T & default_val) : p1(default_val), p2(default_val) {;}
    constexpr ce_array(const ce_array<T,N> & _in) : p1(_in.p1), p2(_in.p2) {;}

    constexpr void operator=(const ce_array<T,N> & _in) { p1 = _in.p1; p2 = _in.p2; }

    constexpr size_t size() const { return N; }
    constexpr bool operator==(const ce_array<T,N> & _in) const {
      return p1 == _in.p1 && p2 == _in.p2;
    }
    constexpr bool operator!=(const ce_array<T,N> & _in) const { return !operator==(_in); }

    constexpr T & operator[](size_t id) { return (id < p1_size) ? p1[id] : p2[id-p1_size]; }
    constexpr const T & operator[](size_t id) const { return (id < p1_size) ? p1[id] : p2[id-p1_size]; }

    constexpr T & back() { return p2.back(); }
    constexpr const T & back() const { return p2.back(); }
    constexpr void fill(const T & v) { p1.fill(v); p2.fill(v); }

    constexpr bool Has(const T & t) const { return p1.Has(t) || p2.Has(t); }

    template <size_t i> constexpr T & Get() {
      static_assert(i < N, "ce_array::Get<id> must have inded in range.");
      return (i < p1_size) ? p1.template Get<i>(0) : p2.template Get<i-p1_size>(0);
    }
    template <size_t i> constexpr const T & Get() const {
      static_assert(i < N, "ce_array::Get<id> must have inded in range.");
      return (i < p1_size) ? p1.template Get<i>(0) : p2.template Get<i-p1_size>(0);
    }
  };

  // Specialized version of ce_array for a single element.
  template <typename T>
  class ce_array<T,1> {
  private:
    T m_val;     // Value at this position.
  public:
    using size_t = std::size_t;
    using value_type = T;

    constexpr ce_array(const T & default_val) : m_val(default_val) {;}
    constexpr ce_array(const ce_array<T,1> & _in) : m_val(_in.m_val) {;}

    constexpr void operator=(const ce_array<T,1> & _in) { m_val = _in.m_val; }

    constexpr size_t size() const { return 1; }
    constexpr bool operator==(const ce_array<T,1> & _in) const { return m_val == _in.m_val; }
    constexpr bool operator!=(const ce_array<T,1> & _in) const { return m_val != _in.m_val; }

    constexpr T & operator[](size_t id) { emp_assert(id==0); return m_val; }
    constexpr const T & operator[](size_t id) const { emp_assert(id==0); return m_val; }

    constexpr T & back() { return m_val; }
    constexpr const T & back() const { return m_val; }
    constexpr void fill(const T & v) { m_val = v; }

    constexpr bool Has(const T & t) const { return m_val == t; }

    template <size_t i> constexpr T & Get() {
      static_assert(i==0, "ce_array::Get<id> must have inded in range.");
      return m_val;
    }
    template <size_t i> constexpr const T & Get() const {
      static_assert(i==0, "ce_array::Get<id> must have inded in range.");
      return m_val;
    }
  };

  // Specialized version of ce_array for an empty array.
  template <typename T>
  class ce_array<T,0> {
  public:
    using size_t = std::size_t;
    using value_type = T;

    constexpr ce_array(const T &) {;}
    constexpr ce_array(const ce_array<T,0> &) {;}

    constexpr void operator=(const ce_array<T,0> &) { ; }

    constexpr size_t size() const { return 0; }
    constexpr bool operator==(const ce_array<T,0> &) const { return true; }
    constexpr bool operator!=(const ce_array<T,0> &) const { return false; }

    // Cannot index into an empty array!
    constexpr T & operator[](size_t id) { emp_assert(false); return *((T*) nullptr); }
    constexpr const T & operator[](size_t id) const { emp_assert(false); return *((T*) nullptr); }

    // No back in an empty array.
    constexpr T & back() { emp_assert(false); return *((T*) nullptr); }
    constexpr const T & back() const { emp_assert(false); return *((T*) nullptr); }
    constexpr void fill(const T &) { ; }

    constexpr bool Has(const T & t) const { return false; }
  };

}

#endif // #ifndef EMP_IN_PROGRESS_CONSTEXPR_CE_ARRAY_HPP_INCLUDE
