/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
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


#ifndef EMP_ARRAY_H
#define EMP_ARRAY_H

#include <algorithm>
#include <initializer_list>
#include <array>
#include <type_traits>

#include "assert.hpp"
#include "../meta/TypeID.hpp"

#ifdef EMP_NDEBUG

namespace emp {
  /// In release mode, emp::array is simply an alias for std::array.
  template <typename T, size_t N> using array = std::array<T,N>;
}

#else

namespace emp {

  /// We are in debug mode, so emp::array has the same interface as std::array, but with extra
  /// bounds checking.  Using vector as our base since it has the right pieces and is dynamic.
  template <typename T, size_t N>
  class array : protected std::vector<T> {
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

      /// Convenience functions to view self as wrapped_t object
      wrapped_t& as_wrapped() { return *static_cast<wrapped_t*>(this); }
      const wrapped_t& as_wrapped() const { return
        *static_cast<const wrapped_t*>(this);
      }

      /// What vector was this iterator created from?
      const vec_t * v_ptr{ nullptr };

      iterator_wrapper() { ; }

      iterator_wrapper(const ITERATOR_T & _in, const vec_t * _v) : ITERATOR_T(_in), v_ptr(_v) { ; }
      iterator_wrapper(const this_t &) = default;
      iterator_wrapper(this_t &&) = default;
      ~iterator_wrapper() { ; }

      // Debug tools to make sure this iterator is okay.
      bool OK(const bool begin_ok=true, const bool end_ok=true) const {
        if (v_ptr == nullptr) return false;                // Invalid vector
        if (!v_ptr->valid) return false;                   // Vector has been deleted!

        int64_t pos = 0;
        if constexpr (
          std::is_same<ITERATOR_T, typename base_t::reverse_iterator>()
          || std::is_same<ITERATOR_T, typename base_t::const_reverse_iterator>()
        ) {
          pos = *((ITERATOR_T *) this) - ((base_t *) v_ptr)->rbegin();
        }
        else {
          pos = *((ITERATOR_T *) this) - ((base_t *) v_ptr)->begin();
        }

        if (pos < 0) {
          // std::cout << "Iterator not allowed past beginning." << '\n';
          return false;
        }

        if (pos > static_cast<int>( v_ptr->size() )) {
          // std::cout << "Iterator out of range." << '\n';
          return false;
        }
        if (!begin_ok && pos == 0) {
          // std::cout << "Iterator not allowed at beginning." << '\n';
          return false;
        }
        if (!end_ok && pos == static_cast<int>(v_ptr->size())) {
          // std::cout << "Iterator not allowed at end." << '\n';
          return false;
        }
        return true;
      }

      this_t & operator=(const this_t &) = default;
      this_t & operator=(this_t &&) = default;

      operator ITERATOR_T() { return *this; }
      operator const ITERATOR_T() const { return *this; }

      // enables the implicit conversion
      // iterator_wrapper<iterator> -> iterator_wrapper<const_iterator>
      template<typename It>
      operator iterator_wrapper<It>() {
        return iterator_wrapper<It>(as_wrapped(), v_ptr);
      }
      template<typename It>
      operator const iterator_wrapper<It>() const {
        return iterator_wrapper<It>(as_wrapped(), v_ptr);
      }

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

      // some stl implementations use a free-function operator+(lhs, rhs)
      // instead of an operator+(rhs) member, so we have to
      //   1. use free-function + syntax
      //      instead of calling wrapped_t::operator+()
      //   2. provide exact type-matching function signatures to prevent
      //      ambiguous overload errors
      // (this problem observed w/ GCC with _GLIBCXX_DEBUG enabled)
      template<
        typename Addend,
        typename = typename std::enable_if<
          std::is_arithmetic<Addend>::value, Addend
        >::type
      >
      auto operator+(const Addend in) {
        emp_assert(OK());
        return this_t(as_wrapped() + in, v_ptr);
      }

      // some stl implementations use a free-function operator-(lhs, rhs)
      // instead of an operator-(rhs) member, so we have to
      //   1. use free-function - syntax
      //      instead of calling wrapped_t::operator-()
      //   2. provide exact type-matching function signatures to prevent
      //      ambiguous overload errors
      // (this problem observed w/ GCC with _GLIBCXX_DEBUG enabled)
      template<
        typename Subtrahend,
        typename = typename std::enable_if<
          std::is_arithmetic<Subtrahend>::value, Subtrahend
        >::type
      >
      auto operator-(Subtrahend in) {
        emp_assert(OK());
        return this_t(as_wrapped() - in, v_ptr);
      }
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

    reverse_iterator rbegin() noexcept {
      return reverse_iterator(base_t::rbegin(), this);
    }
    const_reverse_iterator rbegin() const noexcept {
      return const_reverse_iterator(base_t::rbegin(), this);
    }
    reverse_iterator rend() noexcept {
      return reverse_iterator(base_t::rend(), this);
    }
    const_reverse_iterator rend() const noexcept {
      return const_reverse_iterator(base_t::rend(), this);
    }

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

    // for implicit conversion of iterator -> base_t::const_iterator to work
    // we have to explicitly take const_iterator argument
    template <typename... ARGS>
    iterator insert(const const_iterator pos, ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
      return iterator(base_t::insert(pos, std::forward<ARGS>(args)...), this);
    }

    // for implicit conversion of iterator -> base_t::const_iterator to work
    // we have to explicitly take const_iterator argument
    iterator erase(const const_iterator pos) {
      emp_assert(false, "invalid operation for array!");
      return iterator(base_t::erase(pos), this);
    }

    // for implicit conversion of iterator -> base_t::const_iterator to work
    // we have to explicitly take const_iterator argument
    iterator erase(const const_iterator first, const const_iterator last) {
      emp_assert(false, "invalid operation for array!");
      return iterator(base_t::erase(first, last), this);
    }

    // for implicit conversion of iterator -> base_t::const_iterator to work
    // we have to explicitly take const_iterator argument
    template <typename... ARGS>
    iterator emplace(const const_iterator pos, ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
      return iterator(base_t::emplace(pos, std::forward<ARGS>(args)...), this);
    }

    template <typename... ARGS>
    void emplace_back(ARGS &&... args) {
      emp_assert(false, "invalid operation for array!");
    }

    T* data() { return base_t::data(); }
    const T* data() const { return base_t::data(); }

    T& at(const size_t i) { return base_t::at(i); }
    const T& at(const size_t i) const { return base_t::at(i); }

    bool operator==( const array& other ) const {
      return std::equal(begin(), end(), other.begin());
    }
    bool operator!=( const array& other ) const { return !operator==(other); }

    // previously, when we inherited from std::vector publicly
    // cereal would try to do *vector* serialization (which is bad!)
    // so, we have to write our own array serialization
    template <class Archive>
    void serialize( Archive & ar ) { for (auto & i : *this) ar( i ); }

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

#endif // Include guards
