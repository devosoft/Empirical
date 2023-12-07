/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2022.
 *
 *  @file array.hpp
 *  @brief A drop-in wrapper for std::array; adds on bounds checking in debug mode.
 *  @note Status: RELEASE
 *
 *  If EMP_NDEBUG is set, emp::array is just an alias for std::array.
 *  Otherwise, every time an array is accessed, tests are done to make sure that the
 *  access is legal.
 *
 *  @todo Add tests in array::front and array::back to ensure not empty.
 *  @todo Add tests for get.
 */

#ifndef EMP_BASE_ARRAY_HPP_INCLUDE
#define EMP_BASE_ARRAY_HPP_INCLUDE

#include <array>
#include <initializer_list>
#include <iterator>
#include <stddef.h>
#include <vector>

#include "../../../third-party/cereal/include/cereal/cereal.hpp"

#include "assert.hpp"

#ifdef EMP_NDEBUG

namespace emp {
  /// In release mode, emp::array is simply an alias for std::array.
  template <typename T, size_t N> using array = std::array<T,N>;
}

#else

namespace emp {

  // Pre-declaration of array type.
  template <typename T, size_t N> struct array;

  /// Setup an iterator wrapper to check validity.
  template<typename ITERATOR_T, typename ARRAY_T>
  struct array_iterator {
    using this_t = array_iterator<ITERATOR_T, ARRAY_T>;
    using array_t = ARRAY_T;

    // Iterator traits
    using iterator_category = typename std::iterator_traits<ITERATOR_T>::iterator_category;
    using value_type = typename std::iterator_traits<ITERATOR_T>::value_type;
    using difference_type = typename std::iterator_traits<ITERATOR_T>::difference_type;
    using pointer = typename std::iterator_traits<ITERATOR_T>::pointer;
    using reference = typename std::iterator_traits<ITERATOR_T>::reference;

    ITERATOR_T it;
    const array_t * arr_ptr { nullptr }; // Which array was iterator created from?

    array_iterator() { ; }

    array_iterator(ITERATOR_T _in, const array_t * _v) : it(_in), arr_ptr (_v) { ; }
    array_iterator(const this_t &) = default;
    array_iterator(this_t &&) = default;
    ~array_iterator() { ; }

    // Debug tools to make sure this iterator is okay.
    bool OK(bool begin_ok=true, bool end_ok=true) const {
      if (arr_ptr  == nullptr) return false;                 // Invalid array
      if (it < arr_ptr->begin()) return false;               // Iterator before array start.
      if (it > arr_ptr->end()) return false;                 // Iterator after array end.
      if (!begin_ok && it == arr_ptr->begin()) return false; // Iterator not allowed at start.
      if (!end_ok && it == arr_ptr->end()) return false;     // Iterator not allowed at end.
      return true;
    }

    this_t & operator=(const this_t &) = default;
    this_t & operator=(this_t &&) = default;

    operator ITERATOR_T() { return it; }
    operator const ITERATOR_T() const { return it; }

    auto & operator*() {
      emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
      return *it;
    }
    const auto & operator*() const {
      emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
      return *it;
    }

    auto operator->() {
      emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
      return it;
    }
    auto operator->() const {
      emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
      return it;
    }

    this_t & operator++() { emp_assert(OK(true,false)); ++it; return *this; }
    this_t operator++(int /*x*/) { emp_assert(OK(true,false)); return this_t(it++, arr_ptr); }
    this_t & operator--() { emp_assert(OK(false,true)); --it; return *this; }
    this_t operator--(int /*x*/) { emp_assert(OK(false,true)); return this_t(it--, arr_ptr); }

    this_t operator+(int in) { emp_assert(OK()); return this_t(it + in, arr_ptr); }
    this_t operator-(int in) { emp_assert(OK()); return this_t(it - in, arr_ptr); }
    ptrdiff_t operator-(const this_t & in) { emp_assert(OK()); return it - in.it; }

    this_t & operator+=(int in) { emp_assert(OK()); it += in; return *this; }
    this_t & operator-=(int in) { emp_assert(OK()); it -= in; return *this; }

    auto & operator[](int index) { emp_assert(OK()); return it[index]; }
    const auto & operator[](int index) const { emp_assert(OK()); return it[index]; }
  };

  /// We are in debug mode, so emp::array has the same interface as std::array, but with extra
  /// bounds checking.  Using vector as our base since it has the right pieces and is dynamic.
  template <typename T, size_t NUM_ELEMENTS>
  struct array {
    static constexpr size_t N = NUM_ELEMENTS;
    using this_t = emp::array<T,N>;

    T _data[ N ? N : 1 ];

    using iterator = array_iterator< T*, this_t >;
    using const_iterator = array_iterator< const T *, this_t >;
    using reverse_iterator = array_iterator< std::reverse_iterator<iterator>, this_t >;
    using const_reverse_iterator = array_iterator< std::reverse_iterator<const_iterator>, this_t >;
    using value_type = T;
    using size_type = std::size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    // -- No constructors, destructors, or assignment operators to preserve aggregate type.

    int operator<=>(const array &) const = default;

    // Allow automatic conversion to regular array type.
    operator std::array<T,N>() {
      std::array<T,N> ar;
      for (size_t i = 0; i < N; i++) ar[i] = _data[i];
      return ar;
    }

    constexpr size_t size() const { return N; }

    auto & data() { return _data; }
    const auto & data() const { return _data; }

    iterator begin() noexcept { return iterator(_data, this); }
    const_iterator begin() const noexcept { return const_iterator(_data, this); }
    iterator end() noexcept { return iterator(_data + N, this); }
    const_iterator end() const noexcept { return const_iterator(_data + N, this); }

    T & operator[](size_t pos) {
      emp_assert(pos < N, pos, N);
      return _data[pos];
    }

    const T & operator[](size_t pos) const {
      emp_assert(pos < N, pos, N);
      return _data[pos];
    }

    T & back() { emp_assert(N > 0); return _data[N-1]; }
    const T & back() const { emp_assert(N > 0); return _data[N-1]; }
    T & front() { emp_assert(N > 0); return _data[0]; }
    const T & front() const { emp_assert(N > 0); return _data[0]; }

    void fill(const T & val) {
      for (size_t i = 0; i < N; ++i) _data[i] = val;
    }

    // Functions to make sure to throw an error on:

    void resize(size_t /* new_size */) { emp_assert(false, "invalid operation for array!"); }
    void resize(size_t /* new_size */, const T & /* val */) { emp_assert(false, "invalid operation for array!"); }

    template <typename... PB_Ts>
    void push_back(PB_Ts &&... /* args */) { emp_assert(false, "invalid operation for array!"); }
    void pop_back() { emp_assert(false, "invalid operation for array!"); }

    template <typename... ARGS>
    iterator insert(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    iterator erase(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    iterator emplace(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
      return end();
    }

    template <typename... ARGS>
    void emplace_back(ARGS &&... /* args */) {
      emp_assert(false, "invalid operation for array!");
    }

    template <class Archive>
    void serialize( Archive & ar ) { ar(_data); }
  };


}

// specialization for std::tuple_size
template <class T, size_t N>
struct std::tuple_size<emp::array<T, N>> : public integral_constant<size_t, N> {
};

#endif // NDEBUG off


namespace std {
  // A crude, generic printing function for arrays.
  template <typename T, size_t N>
  inline std::ostream & operator<<(std::ostream & out, const emp::array<T,N> & v) {
    for (const T & x : v) out << x << " ";
    return out;
  }

  template <typename T, size_t N>
  std::istream & operator>>(std::istream & is, emp::array<T,N> & v) {
    for (T & x : v) is >> x;
    return is;
  }

}

#endif // #ifndef EMP_BASE_ARRAY_HPP_INCLUDE
