//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Converted to stand-alone: March 6, 2017
//
//
//  This class is a drop-in wrapper for std::vector, adding on bounds checking.
//  If NDEBUG is set then it reverts back to std::vector.
//
//
//  Developer Notes:
//   * Need an automatic conversion from emp::vector to std::vector and back to interface with
//     non-empirical code.


#ifndef EMP_VECTOR_H
#define EMP_VECTOR_H

#include <assert.h>
#include <initializer_list>
#include <utility>
#include <vector>


#ifdef NDEBUG

// Seemlessly translate emp::vector to std::vector
namespace emp {
  template <typename... Ts> using vector = std::vector<Ts...>;
}


#else // NDEBUG *not* set

namespace emp {

  // Build a debug wrapper emp::vector around std::vector.
  template <typename T, typename... Ts>
  class vector {
  private:
    using this_t = emp::vector<T,Ts...>;
    using stdv_t = std::vector<T,Ts...>;
    stdv_t v;

    // Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2000000001;

  public:
    using iterator = typename stdv_t::iterator;
    using const_iterator = typename stdv_t::const_iterator;
    using value_type = T;
    using size_type = typename stdv_t::size_type;
    using reference = typename stdv_t::reference;
    using const_reference = typename stdv_t::const_reference;

    vector() = default;
    vector(const this_t &) = default;
    vector(size_t size) : v(size) { assert(size < MAX_SIZE); }
    vector(size_t size, const T & val) : v(size, val) { assert(size < MAX_SIZE); }
    vector(std::initializer_list<T> in_list) : v(in_list) { ; }
    vector(const stdv_t & in) : v(in) { ; }         // Emergency fallback conversion.
    ~vector() = default;

    operator stdv_t &() { return v; }
    operator const stdv_t &() const { return v; }

    size_type size() const noexcept { return v.size(); }
    void clear() { v.clear(); }
    void resize(size_t new_size) { assert(new_size < MAX_SIZE); v.resize(new_size); }
    void resize(size_t new_size, const T & val) {
      assert(new_size < MAX_SIZE);
      v.resize(new_size, val);
    }
    bool empty() const noexcept { return v.empty(); }
    size_type capacity() const noexcept { return v.capacity(); }
    size_type max_size() const noexcept { return v.max_size(); }
    void reserve(size_type n) { v.reserve(n); }
    void shrink_to_fit() { v.shrink_to_fit(); }

    this_t & operator=(const emp::vector<T,Ts...> &) = default;
    this_t & operator=(const stdv_t & x) { v = x; return *this; }
    this_t & operator=(const std::initializer_list<value_type> & il) {
      v.operator=(il);
      return *this;
    }

    void swap(this_t & x) { v.swap(x.v); }
    void swap(stdv_t & x) { v.swap(x); }

    bool operator==(const this_t & in) const { return v == in.v; }
    bool operator!=(const this_t & in) const { return v != in.v; }
    bool operator<(const this_t & in)  const { return v < in.v; }
    bool operator<=(const this_t & in) const { return v <= in.v; }
    bool operator>(const this_t & in)  const { return v > in.v; }
    bool operator>=(const this_t & in) const { return v >= in.v; }

    T & operator[](size_t pos) {
      assert(pos < v.size());
      return v[pos];
    }

    const T & operator[](size_t pos) const {
      assert(pos < v.size());
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

    T & back() { assert(size() > 0); return v.back(); }
    const T & back() const { assert(size() > 0); return v.back(); }
    T & front() { assert(size() > 0); return v.front(); }
    const T & front() const { assert(size() > 0); return v.front(); }

    void pop_back() {
      assert(v.size() > 0);
      v.pop_back();
    }
  };

}

// A crude, generic printing function for vectors.
template <typename T, typename... Ts>
std::ostream & operator<<(std::ostream & out, const emp::vector<T,Ts...> & v) {
  for (const T & x : v) out << x << " ";
  return out;
}

template <typename T, typename... Ts>
std::istream & operator>>(std::istream & is, emp::vector<T,Ts...> & v) {
  for (T & x : v) is >> x;
  return is;
}

#endif  // NDEBUG not defined

#endif  // Include guard
