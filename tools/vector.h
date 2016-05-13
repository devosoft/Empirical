//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class is a drop-in wrapper for std::vector, adding on bounds checking.
//  If EMP_NDEBUG is set then it reverts back to std::vector.


#ifndef EMP_VECTOR_H
#define EMP_VECTOR_H

#include <initializer_list>
#include <utility>
#include <vector>

#include "assert.h"


#ifdef EMP_NDEBUG

namespace emp {
  template <typename T>
    using vector = std::vector<T>;
}


#else // #EMP_NDEBUG *not* set

namespace emp {

  template <typename T>
  class vector {
  private:
    std::vector<T> v;

  public:
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using value_type = T;

    vector() = default;
    vector(const emp::vector<T> &) = default;
    vector(int size) : v(size) { emp_assert(size >= 0, size); }
    vector(int size, const T & val) : v(size, val) { emp_assert(size >= 0, size); }
    vector(std::initializer_list<T> in_list) : v(in_list) { ; }
    vector(const std::vector<T> & in) : v(in) { ; }         // Emergency fallback conversion.
    ~vector() = default;

    uint32_t size() const { return v.size(); }
    void resize(int new_size) { emp_assert(new_size >= 0, new_size); v.resize(new_size); }
    void resize(int new_size, const T & val) {
      emp_assert(new_size >= 0, new_size);
      v.resize(new_size, val);
    }

    emp::vector<T> & operator=(const emp::vector<T> &) = default;

    bool operator==(const emp::vector<T> & in) const { return v == in.v; }
    bool operator!=(const emp::vector<T> & in) const { return v != in.v; }
    bool operator<(const emp::vector<T> & in)  const { return v < in.v; }
    bool operator<=(const emp::vector<T> & in) const { return v <= in.v; }
    bool operator>(const emp::vector<T> & in)  const { return v > in.v; }
    bool operator>=(const emp::vector<T> & in) const { return v >= in.v; }

    T & operator[](int pos) {
      emp_assert(pos >= 0 && pos < (int) v.size(), pos, v.size());
      return v[pos];
    }

    const T & operator[](int pos) const {
      emp_assert(pos >= 0 && pos < (int) v.size(), pos, v.size());
      return v[pos];
    }

    template <typename T2>
    vector & push_back(T2 && in) { v.push_back(std::forward<T2>(in)); return *this; }

    template <typename... T2>
    vector & emplace_back(T2 &&... in) { v.emplace_back(std::forward<T2>(in)...); return *this; }

    auto begin() -> decltype(v.begin()) { return v.begin(); }
    auto end() -> decltype(v.end()) { return v.end(); }
    auto begin() const -> const decltype(v.begin()) { return v.begin(); }
    auto end() const -> const decltype(v.end()) { return v.end(); }

    auto rbegin() -> decltype(v.rbegin()) { return v.rbegin(); }
    auto rend() -> decltype(v.rend()) { return v.rend(); }
    auto rbegin() const -> const decltype(v.rbegin()) { return v.rbegin(); }
    auto rend() const -> const decltype(v.rend()) { return v.rend(); }

    template <typename... ARGS>
    iterator insert(ARGS &&... args) { return v.insert(std::forward<ARGS>(args)...); }


    T & back() { return v.back(); }
    const T & back() const { return v.back(); }

    void pop_back() {
      emp_assert(v.size() > 0, v.size());
      v.pop_back();
    }
  };

}

// A crude, generic printing function for vectors.
template <typename T> std::ostream & operator<<(std::ostream & out, const emp::vector<T> & v) {
  for (const T & x : v) out << x << " ";
  return out;
}

template <typename T> std::istream & operator>>(std::istream & is, emp::vector<T> & v) {
  for (T & x : v) is >> x;
  return is;
}

#endif



#endif
