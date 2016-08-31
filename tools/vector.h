//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  This class is a drop-in wrapper for std::vector, adding on bounds checking.
//  If EMP_NDEBUG is set then it reverts back to std::vector.
//
//
//  Developer Notes:
//   * Need an automatic conversion from emp::vector to std::vector and back to interface with
//     non-empirical code.


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
    using size_type = typename std::vector<T>::size_type;
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    vector() = default;
    vector(const emp::vector<T> &) = default;
    vector(int size) : v(size) { emp_assert(size >= 0, size); }
    vector(int size, const T & val) : v(size, val) { emp_assert(size >= 0, size); }
    vector(std::initializer_list<T> in_list) : v(in_list) { ; }
    vector(const std::vector<T> & in) : v(in) { ; }         // Emergency fallback conversion.
    ~vector() = default;

    operator std::vector<T> &() { return v; }
    operator const std::vector<T> &() const { return v; }

    size_type size() const noexcept { return v.size(); }
    void clear() { v.clear(); }
    void resize(int new_size) { emp_assert(new_size >= 0, new_size); v.resize(new_size); }
    void resize(int new_size, const T & val) {
      emp_assert(new_size >= 0, new_size);
      v.resize(new_size, val);
    }
    bool empty() const noexcept { return v.empty(); }
    size_type capacity() const noexcept { return v.capacity(); }
    size_type max_size() const noexcept { return v.max_size(); }
    void reserve(size_type n) { v.reserve(n); }
    void shrink_to_fit() { v.shrink_to_fit(); }

    emp::vector<T> & operator=(const emp::vector<T> &) = default;
    emp::vector<T> & operator=(const std::vector<T> & x) { v = x; return *this; }
    emp::vector<T> & operator=(const std::initializer_list<value_type> & il) {
      v.operator=(il);
      return *this;
    }

    void swap(emp::vector<T> & x) { v.swap(x.v); }
    void swap(std::vector<T> & x) { v.swap(x); }

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

    T* data() noexcept { return v.data(); }
    const T* data() const noexcept { return v.data(); }

    template <typename T2>
    vector & push_back(T2 && in) { v.push_back(std::forward<T2>(in)); return *this; }

    template <typename... T2>
    iterator emplace (T2 &&... in) { return v.emplace_back(std::forward<T2>(in)...); }
    template <typename... T2>
    void emplace_back(T2 &&... in) { v.emplace_back(std::forward<T2>(in)...); }

    template <typename... T2>
    iterator erase(T2 &&... in) { return v.erase(std::forward<T2>(in)...); }

    auto begin() -> decltype(v.begin()) { return v.begin(); }
    auto end() -> decltype(v.end()) { return v.end(); }
    auto begin() const -> const decltype(v.begin()) { return v.begin(); }
    auto end() const -> const decltype(v.end()) { return v.end(); }

    auto rbegin() -> decltype(v.rbegin()) { return v.rbegin(); }
    auto rend() -> decltype(v.rend()) { return v.rend(); }
    auto rbegin() const -> const decltype(v.rbegin()) { return v.rbegin(); }
    auto rend() const -> const decltype(v.rend()) { return v.rend(); }

    auto cbegin() const noexcept -> decltype(v.cbegin()) { return v.cbegin(); }
    auto cend() const noexcept -> decltype(v.cend()) { return v.cend(); }
    auto crbegin() const noexcept -> const decltype(v.crbegin()) { return v.crbegin(); }
    auto crend() const noexcept -> const decltype(v.crend()) { return v.crend(); }

    template <typename... ARGS>
    void assign(ARGS &&... args) { v.assign(std::forward<ARGS>(args)...); }

    reference at(size_type n) { return v.at(n); }
    const_reference at(size_type n) const { return v.at(n); }

    template <typename... ARGS>
    iterator insert(ARGS &&... args) { return v.insert(std::forward<ARGS>(args)...); }

    T & back() { emp_assert(size() > 0); return v.back(); }
    const T & back() const { emp_assert(size() > 0); return v.back(); }
    T & front() { emp_assert(size() > 0); return v.front(); }
    const T & front() const { emp_assert(size() > 0); return v.front(); }

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
