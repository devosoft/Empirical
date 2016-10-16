//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  ce_string defines a limited array object for use within a constexpr class or function.
//
//  Note that this class will be slow if used at run-time without conversion to an
//  array or vector first.

#ifndef EMP_CE_ARRAY_H
#define EMP_CE_ARRAY_H

#include <type_traits>

#include "assert.h"
#include "functions.h"

namespace emp {

  template <typename T, size_t N>
  class ce_array {
  private:
    T value;                 // Current value.
    ce_array<T, N-1> next;   // Additional values.

  public:
    using size_t = std::size_t;
    using value_type = T;

    constexpr ce_array(const T & default_val) : value(default_val), next(default_val) {;}
    constexpr ce_array(const ce_array<T,N> & _in) : value(_in.value), next(_in.next) {;}

    constexpr void operator=(const ce_array<T,N> & _in) {
      value = _in.value;
      next = _in.next;
    }

    constexpr size_t size() const { return N; }
    constexpr bool operator==(const ce_array<T,N> & _in) const {
      return (value==_in.value) ? (next==_in.next) : false;
    }
    constexpr bool operator!=(const ce_array<T,N> & _in) const { return !operator==(_in); }

    constexpr T & operator[](int id) { return (id==0) ? value : next.operator[](id-1); }
    constexpr const T & operator[](int id) const { return (id==0) ? value : next.operator[](id-1); }

    constexpr T & back() { return operator[](N-1); }
    constexpr const T & back() const { return operator[](N-1); }
    constexpr void fill(const T & v) { value = v; next.fill(v); }

    constexpr bool Has(const T & t) const { return (value == t) || next.Has(t); }
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
    constexpr T & operator[](int id) { emp_assert(false); return *((T*) nullptr); }
    constexpr const T & operator[](int id) const { emp_assert(false); return *((T*) nullptr); }

    // No back in an empty array.
    constexpr T & back() { emp_assert(false); return *((T*) nullptr); }
    constexpr const T & back() const { emp_assert(false); return *((T*) nullptr); }
    constexpr void fill(const T &) { ; }

    constexpr bool Has(const T & t) const { return false; }
  };

}

#endif
