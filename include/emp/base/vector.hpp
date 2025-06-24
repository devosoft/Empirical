/**
 * This file is part of Empirical, https://github.com/devosoft/Empirical
 * Copyright (C) 2016-2019 Michigan State University
 * MIT Software license; see doc/LICENSE.md
 *
 * @file include/emp/base/vector.hpp
 * @brief A drop-in wrapper for std::vector; adds on bounds checking in debug mode.
 * @note Status: BETA
 *
 * This class is a drop-in wrapper for std::vector, adding on bounds checking, both for the
 * indexing operator and for the use of iterators (ensure that iterators do not outlive the
 * version of vector for which it was created.)
 * If NDEBUG is set then it reverts back to std::vector.
 *
 * @todo Debug code: member functions that take iterators should also take emp iterators that verify
 *       whether those iterators are valid.
 */

#pragma once

#ifndef INCLUDE_EMP_BASE_VECTOR_HPP_GUARD
#define INCLUDE_EMP_BASE_VECTOR_HPP_GUARD


#include <initializer_list>
#include <iterator>
#include <stddef.h>
#include <utility>
#include <vector>

#include "assert.hpp"

namespace emp {

  /// Build a wrapper for emp::vector around std::vector.
  template <typename T, typename... Ts>
  class vector : public std::vector<T, Ts...> {
  private:
    using this_t = emp::vector<T, Ts...>;
    using stdv_t = std::vector<T, Ts...>;

#ifndef NDEBUG
    /// Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2'000'000'001;  // 2x10^9 + 1

    /// Setup a revision number - iterators must match the revision of their vector.
    int revision = 0;
#endif  // #ifndef NDEBUG

  public:
#ifdef NDEBUG
    // Outside of debug mode, we just need to make constructors available.
    vector() : stdv_t() {}

    vector(const this_t & _in) : stdv_t(_in) {}

    vector(size_t size) : stdv_t(size) {}

    vector(size_t size, const T & val) : stdv_t(size, val) {}

    vector(std::initializer_list<T> in_list) : stdv_t(in_list) {}

    vector(const stdv_t & in) : stdv_t(in) {}  // Emergency fallback conversion.

    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last) {}

    this_t & operator=(const this_t &) & = default;
    this_t & operator=(this_t &&) &      = default;

#else   // #ifdef NDEBUG

    /// Setup an iterator wrapper to make sure that they're not used again after a vector changes.
    template <typename ITERATOR_T>
    struct iterator_wrapper : public ITERATOR_T {
      using this_it_t = iterator_wrapper<ITERATOR_T>;
      using wrapped_t = ITERATOR_T;
      using vec_t     = emp::vector<T, Ts...>;

      /// What vector and revision was this iterator created from?
      const vec_t * v_ptr;
      int revision;

      // @CAO: For the moment cannot create an emp iterator from a base since we don't know vector to use.
      // iterator_wrapper(const ITERATOR_T & _in)
      //   : ITERATOR_T(_in), v_ptr(nullptr), revision(0) { ; }
      iterator_wrapper(const ITERATOR_T & _in, const vec_t * _v)
        : ITERATOR_T(_in), v_ptr(_v), revision(_v->revision) {
        ;
      }

      iterator_wrapper(const this_it_t &) = default;
      iterator_wrapper(this_it_t &&)      = default;
      iterator_wrapper()                  = default;

      ~iterator_wrapper() { ; }

      // Debug tools to make sure this iterator is okay.
      static std::string & ErrorCode() {
        static std::string code = "No Errors Found.";
        return code;
      }

      static std::string ErrorStart() {
        std::string vec_type =
          std::string("vector<") + typeid(typename stdv_t::value_type).name() + ">";
        std::string it_type = typeid(ITERATOR_T).name();
        if constexpr (std::is_same<ITERATOR_T, typename stdv_t::iterator>()) {
          it_type = "iterator";
        }
        if constexpr (std::is_same<ITERATOR_T, typename stdv_t::const_iterator>()) {
          it_type = "const_iterator";
        }
        if constexpr (std::is_same<ITERATOR_T, typename stdv_t::reverse_iterator>()) {
          it_type = "reverse_iterator";
        }
        if constexpr (std::is_same<ITERATOR_T, typename stdv_t::const_reverse_iterator>()) {
          it_type = "const_reverse_iterator";
        }
        return std::string("Iterator (type = '") + vec_type + "::" + it_type + "') ";
      }

      bool OK(bool begin_ok = true, bool end_ok = true, std::string op = "") const {
        // std::string type_name = typeid(ITERATOR_T).name();

        if (v_ptr == nullptr) {
          ErrorCode() = "Invalid Vector! (set to nullptr)";
          return false;
        }
        if (v_ptr->revision == 0) {
          ErrorCode() = "Vector deleted! (revision==0)";
          return false;
        }
        if (revision != v_ptr->revision) {
          ErrorCode() = "Vector has changed memory!";
          return false;
        }

        int64_t pos = 0;
        if constexpr (std::is_same<ITERATOR_T, typename stdv_t::reverse_iterator>() ||
                      std::is_same<ITERATOR_T, typename stdv_t::const_reverse_iterator>()) {
          // pos = std::distance(*((ITERATOR_T *) this), ((stdv_t *) v_ptr)->rend()) - 1;
          pos = ((stdv_t *) v_ptr)->rend() - *((ITERATOR_T *) this) - 1;
        } else {
          // pos = std::distance(((stdv_t *) v_ptr)->begin(), *((ITERATOR_T *) this));
          pos = *((ITERATOR_T *) this) - ((stdv_t *) v_ptr)->begin();
        }
        if (pos < 0 || ((size_t) pos) > v_ptr->size()) {
          ErrorCode() = ErrorStart() + "out of range." + " size=" + std::to_string(v_ptr->size()) +
                        "  pos=" + std::to_string(pos);
          return false;
        }
        if (!begin_ok && pos == 0) {
          ErrorCode() = ErrorStart() + "not allowed at begin() for operation " + op + ".";
          return false;
        }
        if (!end_ok && ((size_t) pos) == v_ptr->size()) {
          ErrorCode() = ErrorStart() + "not allowed at end() for operation " + op + ".";
          return false;
        }
        return true;
      }

      this_it_t & operator=(const this_it_t &) & = default;
      this_it_t & operator=(this_it_t &&) &      = default;

      operator ITERATOR_T() { return *this; }

      operator const ITERATOR_T() const { return *this; }

      auto & operator*() {
        emp_assert(OK(true, false, "dereference"), ErrorCode());
        return wrapped_t::operator*();
      }

      const auto & operator*() const {
        emp_assert(OK(true, false, "const dereference"), ErrorCode());
        return wrapped_t::operator*();
      }

      auto operator->() {
        //        emp_assert(OK(true, false, "->"), ErrorCode());
        emp_assert(
          OK(true, true, "->"),
          ErrorCode());  // Technically can use -> on end() for memory identification, just can't use result.
        return wrapped_t::operator->();
      }

      auto operator->() const {
        //        emp_assert(OK(true, false, "const ->"), ErrorCode());
        emp_assert(
          OK(true, true, "const ->"),
          ErrorCode());  // Technically can use -> on end() for memory identification, just can't use result.
        return wrapped_t::operator->();
      }

      this_it_t & operator++() {
        emp_assert(OK(true, false, "++ (post)"), ErrorCode());
        wrapped_t::operator++();
        return *this;
      }

      this_it_t operator++(int x) {
        emp_assert(OK(true, false, "++ (pre)"), ErrorCode());
        return this_it_t(wrapped_t::operator++(x), v_ptr);
      }

      this_it_t & operator--() {
        emp_assert(OK(false, true, "-- (post)"), ErrorCode());
        wrapped_t::operator--();
        return *this;
      }

      this_it_t operator--(int x) {
        emp_assert(OK(false, true, "-- (pre)"), ErrorCode());
        return this_it_t(wrapped_t::operator--(x), v_ptr);
      }

      auto operator+(int in) {
        emp_assert(OK(), ErrorCode());
        return this_it_t(wrapped_t::operator+(in), v_ptr);
      }

      auto operator-(int in) {
        emp_assert(OK(), ErrorCode());
        return this_it_t(wrapped_t::operator-(in), v_ptr);
      }

      auto operator-(const this_it_t & in) {
        emp_assert(OK(), ErrorCode());
        return ((wrapped_t) * this) - (wrapped_t) in;
      }

      this_it_t & operator+=(int in) {
        emp_assert(OK(), ErrorCode());
        wrapped_t::operator+=(in);
        return *this;
      }

      this_it_t & operator-=(int in) {
        emp_assert(OK(), ErrorCode());
        wrapped_t::operator-=(in);
        return *this;
      }

      auto & operator[](int offset) {
        emp_assert(OK(), ErrorCode());
        return wrapped_t::operator[](offset);
      }
    };  // struct iterator_wrapper

    using iterator               = iterator_wrapper<typename stdv_t::iterator>;
    using const_iterator         = iterator_wrapper<typename stdv_t::const_iterator>;
    using reverse_iterator       = iterator_wrapper<typename stdv_t::reverse_iterator>;
    using const_reverse_iterator = iterator_wrapper<typename stdv_t::const_reverse_iterator>;
    using value_type             = T;
    using size_type              = typename stdv_t::size_type;
    using reference              = typename stdv_t::reference;
    using const_reference        = typename stdv_t::const_reference;

    vector() : stdv_t(), revision(1){};
    vector(const this_t & _in) : stdv_t(_in), revision(1){};

    vector(size_t size) : stdv_t(size), revision(1) { emp_assert(size < MAX_SIZE, size); }

    vector(size_t size, const T & val) : stdv_t(size, val), revision(1) {
      emp_assert(size < MAX_SIZE, size);
    }

    vector(std::initializer_list<T> in_list) : stdv_t(in_list), revision(1) { ; }

    vector(const stdv_t & in) : stdv_t(in), revision(1) { ; }  // Emergency fallback conversion.

    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last), revision(1) {
      ;
    }

    ~vector() { revision = 0; }  // Clear out revision when vector is deleted.

    size_t size() const { return stdv_t::size(); }

    iterator begin() noexcept { return iterator(stdv_t::begin(), this); }

    const_iterator begin() const noexcept { return const_iterator(stdv_t::begin(), this); }

    iterator end() noexcept { return iterator(stdv_t::end(), this); }

    const_iterator end() const noexcept { return const_iterator(stdv_t::end(), this); }

    const_iterator cbegin() const noexcept { return const_iterator(stdv_t::cbegin(), this); }

    const_iterator cend() const noexcept { return const_iterator(stdv_t::cend(), this); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(stdv_t::rbegin(), this); }

    const_reverse_iterator rbegin() const noexcept {
      return const_reverse_iterator(stdv_t::rbegin(), this);
    }

    reverse_iterator rend() noexcept { return reverse_iterator(stdv_t::rend(), this); }

    const_reverse_iterator rend() const noexcept {
      return const_reverse_iterator(stdv_t::rend(), this);
    }

    const_reverse_iterator crbegin() const noexcept {
      return const_reverse_iterator(stdv_t::crbegin(), this);
    }

    const_reverse_iterator crend() const noexcept {
      return const_reverse_iterator(stdv_t::crend(), this);
    }

    // operator stdv_t &() { return v; }
    // operator const stdv_t &() const { return v; }

    void resize(size_t new_size) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size);
      revision++;
    }

    void resize(size_t new_size, const T & val) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size, val);
      revision++;
    }

    this_t & operator=(const this_t &) & = default;
    this_t & operator=(this_t &&) &      = default;

    T & operator[](size_t pos) {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    const T & operator[](size_t pos) const {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    T & back() {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::back();
    }

    const T & back() const {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::back();
    }

    T & front() {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::front();
    }

    const T & front() const {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::front();
    }

    template <typename... PB_Ts>
    void push_back(PB_Ts &&... args) {
      stdv_t::push_back(std::forward<PB_Ts>(args)...);
      revision++;
    }

    void pop_back() {
      emp_assert(stdv_t::size() > 0, stdv_t::size());
      stdv_t::pop_back();
      revision++;  // Technically reducing size can cause memory reallocation, but less likely.
    }

    template <typename... ARGS>
    iterator insert(ARGS &&... args) {
      ++revision;
      return iterator(stdv_t::insert(std::forward<ARGS>(args)...), this);
    }

    template <typename... ARGS>
    iterator erase(ARGS &&... args) {
      ++revision;
      return iterator(stdv_t::erase(std::forward<ARGS>(args)...), this);
    }

    template <typename... ARGS>
    iterator emplace(ARGS &&... args) {
      ++revision;
      return iterator(stdv_t::emplace(std::forward<ARGS>(args)...), this);
    }

    template <typename... ARGS>
    void emplace_back(ARGS &&... args) {
      stdv_t::emplace_back(std::forward<ARGS>(args)...);
      revision++;
    }
#endif  // #ifdef NDEBUG : #else

  };  // class vector

  /// Build a specialized debug wrapper for emp::vector<bool>
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  template <typename... Ts>
  class vector<bool, Ts...>
    : public std::vector<bool, Ts...>
#else   // #ifndef DOXYGEN_SHOULD_SKIP_THIS
  template <typename t>
  class vector<bool>
#endif  // #ifndef DOXYGEN_SHOULD_SKIP_THIS : #else
  {
  private:
    using this_t = emp::vector<bool, Ts...>;
    using stdv_t = std::vector<bool, Ts...>;

#ifndef NDEBUG
    /// Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2'000'000'001;
#endif  // #ifndef NDEBUG

  public:
    using iterator        = typename stdv_t::iterator;
    using const_iterator  = typename stdv_t::const_iterator;
    using value_type      = bool;
    using size_type       = typename stdv_t::size_type;
    using reference       = typename stdv_t::reference;
    using const_reference = typename stdv_t::const_reference;

    vector()                   = default;
    vector(const this_t & _in) = default;
    vector(this_t && _in)      = default;

    vector(size_t size) : stdv_t(size) { emp_assert(size < MAX_SIZE, size); }

    vector(size_t size, bool val) : stdv_t(size, val) { emp_assert(size < MAX_SIZE, size); }

    vector(std::initializer_list<bool> in_list) : stdv_t(in_list) { ; }

    vector(const stdv_t & in) : stdv_t(in) { ; }  // Emergency fallback conversion.

    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last) {
      ;
    }

#ifndef NDEBUG
    // In debug mode, wrap functions important to test.

    // operator stdv_t &() { return v; }
    // operator const stdv_t &() const { return v; }

    void resize(size_t new_size) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size);
    }

    void resize(size_t new_size, bool val) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size, val);
    }

    this_t & operator=(const this_t &) & = default;
    this_t & operator=(this_t &&) &      = default;

    auto operator[](size_t pos) -> decltype(stdv_t::operator[](pos)) {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    bool operator[](size_t pos) const {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    auto & back() {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::back();
    }

    bool back() const {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::back();
    }

    auto & front() {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::front();
    }

    bool front() const {
      emp_assert(stdv_t::size() > 0);
      return stdv_t::front();
    }

    void pop_back() {
      emp_assert(stdv_t::size() > 0, stdv_t::size());
      stdv_t::pop_back();
    }
#endif  // #ifndef NDEBUG
  };

  // A crude, generic printing function for vectors.
  template <typename T, typename... Ts>
  std::ostream & operator<<(std::ostream & out, const emp::vector<T, Ts...> & v) {
    for (const T & x : v) { out << x << " "; }
    return out;
  }

  template <typename T, typename... Ts>
  std::istream & operator>>(std::istream & is, emp::vector<T, Ts...> & v) {
    for (T & x : v) { is >> x; }
    return is;
  }

}  // namespace emp

#endif  // #ifndef INCLUDE_EMP_BASE_VECTOR_HPP_GUARD

// Local settings for Empecable file checker.
// empecable_words: stdv
