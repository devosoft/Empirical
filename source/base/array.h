/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file array.h
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


#ifndef EMP_ARRAY_H
#define EMP_ARRAY_H

#include <initializer_list>
#include <array>

#include "assert.h"
#include "../meta/TypeID.h"

#ifdef EMP_NDEBUG

namespace emp {
  /// In release mode, emp::array is simply an alias for std::array.
  template <typename T, size_t N> using array = std::array<T,N>;
}

#else

namespace emp {

  /// We are in debug mode, so emp::array has the same interface as std::array, but with extra
  /// bounds checking.  Using vector as our base since it has the right pieces and is dyanmic.
  template <typename T, size_t N>
  class array : public std::vector<T> {
  private:
    using this_t = emp::array<T,N>;
    using base_t = std::vector<T>;

  public:
    bool valid;

    /// Setup an iterator wrapper to make sure that they're valid.
    template<typename ITERATOR_T>
    struct iterator_wrapper : public ITERATOR_T {
      using this_t = iterator_wrapper<ITERATOR_T>;
      using wrapped_t = ITERATOR_T;
      using vec_t = emp::array<T,N>;

      /// What vector was this iterator created from?
      const vec_t * v_ptr;

      iterator_wrapper(const ITERATOR_T & _in, const vec_t * _v) : ITERATOR_T(_in), v_ptr(_v) { ; }
      iterator_wrapper(const this_t &) = default;
      iterator_wrapper(this_t &&) = default;
      ~iterator_wrapper() { ; }

      // Debug tools to make sure this iterator is okay.
      bool OK(bool begin_ok=true, bool end_ok=true) const {
        if (v_ptr == nullptr) return false;                // Invalid vector
        if (!v_ptr->valid) return false;                   // Vector has been deleted!
        size_t pos = (size_t) (*this - v_ptr->begin());
        if (pos > v_ptr->size()) return false;             // Iterator out of range.
        if (!begin_ok && pos == 0) return false;           // Iterator not allowed at beginning.
        if (!end_ok && pos == v_ptr->size()) return false; // Iterator not allowed at end.
        return true;
      }

      this_t & operator=(const this_t &) = default;
      this_t & operator=(this_t &&) = default;

      operator ITERATOR_T() { return *this; }
      operator const ITERATOR_T() const { return *this; }

      auto & operator*() {
        emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
        return wrapped_t::operator*();
      }
      const auto & operator*() const {
        emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
        return wrapped_t::operator*();
      }

      auto operator->() {
        emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
        return wrapped_t::operator->();
      }
      const auto operator->() const {
        emp_assert(OK(true, false));  // Ensure array is being pointed to properly.
        return wrapped_t::operator->();
      }

      this_t & operator++() { emp_assert(OK(true,false)); wrapped_t::operator++(); return *this; }
      this_t operator++(int x) { emp_assert(OK(true,false)); return this_t(wrapped_t::operator++(x), v_ptr); }
      this_t & operator--() { emp_assert(OK(false,true)); wrapped_t::operator--(); return *this; }
      this_t operator--(int x) { emp_assert(OK(false,true)); return this_t(wrapped_t::operator--(x), v_ptr); }

      auto operator+(int in) { emp_assert(OK()); return this_t(wrapped_t::operator+(in), v_ptr); }
      auto operator-(int in) { emp_assert(OK()); return this_t(wrapped_t::operator-(in), v_ptr); }
      auto operator-(const this_t & in) { emp_assert(OK()); return ((wrapped_t) *this) - (wrapped_t) in; }

      this_t & operator+=(int in) { emp_assert(OK()); wrapped_t::operator+=(in); return *this; }
      this_t & operator-=(int in) { emp_assert(OK()); wrapped_t::operator-=(in); return *this; }
      auto & operator[](int offset) { emp_assert(OK()); return wrapped_t::operator[](offset); }
    };

    using iterator = iterator_wrapper< typename base_t::iterator >;
    using const_iterator = iterator_wrapper< typename base_t::const_iterator >;
    using reverse_iterator = iterator_wrapper< typename base_t::reverse_iterator >;
    using const_reverse_iterator = iterator_wrapper< typename base_t::const_reverse_iterator >;
    using value_type = T;
    using size_type = typename base_t::size_type;
    using reference = typename base_t::reference;
    using const_reference = typename base_t::const_reference;

    array() : base_t(N), valid(true) {};
    array(const this_t & _in) : base_t(_in), valid(true) { emp_assert(_in.size() == N); };
    array(std::initializer_list<T> in_list) : base_t(in_list), valid(true) { emp_assert(size() == N); }
    template <typename InputIt>
    array(InputIt first, InputIt last) : base_t(first, last), valid(true) { emp_assert(size() == N); }
    ~array() { valid=false; } // No longer valid when array is deleted.

    operator std::array<T,N>() {
      std::array<T,N> ar;
      for (size_t i = 0; i < N; i++) ar[i] = base_t::operator[](i);
      return ar;
    }

    constexpr size_t size() const { return N; }

    iterator begin() noexcept { return iterator(base_t::begin(), this); }
    const_iterator begin() const noexcept { return const_iterator(base_t::begin(), this); }
    iterator end() noexcept { return iterator(base_t::end(), this); }
    const_iterator end() const noexcept { return const_iterator(base_t::end(), this); }

    this_t & operator=(const this_t &) = default;

    T & operator[](size_t pos) {
      emp_assert(pos < N, pos, N);
      return base_t::operator[](pos);
    }

    const T & operator[](size_t pos) const {
      emp_assert(pos < N, pos, N);
      return base_t::operator[](pos);
    }

    T & back() { emp_assert(N > 0); return base_t::back(); }
    const T & back() const { emp_assert(N > 0); return base_t::back(); }
    T & front() { emp_assert(N > 0); return base_t::front(); }
    const T & front() const { emp_assert(N > 0); return base_t::front(); }

    void fill(const T & val) { this->assign(N, val); }

    // Functions to make sure to throw an error on:

    void resize(size_t new_size) { emp_assert(false, "invalid operation for array!"); }
    void resize(size_t new_size, const T & val) { emp_assert(false, "invalid operation for array!"); }

    template <typename... PB_Ts>
    void push_back(PB_Ts &&... args) { emp_assert(false, "invalid operation for array!"); }
    void pop_back() { emp_assert(false, "invalid operation for array!"); }

    template <typename... ARGS>
    iterator insert(ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
      return iterator( base_t::insert(std::forward<ARGS>(args)...), this );
    }

    template <typename... ARGS>
    iterator erase(ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
      return iterator( base_t::erase(std::forward<ARGS>(args)...), this );
    }

    template <typename... ARGS>
    iterator emplace(ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
      return iterator( base_t::emplace(std::forward<ARGS>(args)...), this );
    }

    template <typename... ARGS>
    void emplace_back(ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
    }
  };


}

// A crude, generic printing function for arrays.
template <typename T, size_t N>
std::ostream & operator<<(std::ostream & out, const emp::array<T,N> & v) {
  for (const T & x : v) out << x << " ";
  return out;
}

template <typename T, size_t N>
std::istream & operator>>(std::istream & is, emp::array<T,N> & v) {
  for (T & x : v) is >> x;
  return is;
}

#endif

#endif
