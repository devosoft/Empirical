/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file vector.h
 *  @brief A drop-in wrapper for std::vector; adds on bounds checking in debug mode.
 *  @note Status: BETA
 *
 *  This class is a drop-in wrapper for std::vector, adding on bounds checking.
 *  If EMP_NDEBUG is set then it reverts back to std::vector.
 *
 *  @todo Need an automatic conversion from emp::vector to std::vector and back to interface with
 *     non-empirical code.
 */


#ifndef EMP_VECTOR_H
#define EMP_VECTOR_H

#include <initializer_list>
#include <iterator>
#include <utility>
#include <vector>

#include "assert.h"

#ifdef EMP_NDEBUG

// Seemlessly translate emp::vector to std::vector
namespace emp {
  template <typename... Ts> using vector = std::vector<Ts...>;
}


#else // #EMP_NDEBUG *not* set

namespace emp {

  /// Build a debug wrapper emp::vector around std::vector.
  template <typename T, typename... Ts>
  class vector : public std::vector<T,Ts...> {
  private:
    using this_t = emp::vector<T,Ts...>;
    using stdv_t = std::vector<T,Ts...>;

    /// Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2000000001; // 2x10^9 + 1

  public:
    /// Setup a revision number - iterators must match the revision of their vector.
    int revision;

    /// Setup an iterator wrapper to make sure that they're not used again after a vector changes.
    template<typename ITERATOR_T>
    struct iterator_wrapper : public ITERATOR_T {
      using this_t = iterator_wrapper<ITERATOR_T>;
      using wrapped_t = ITERATOR_T;
      using vec_t = emp::vector<T,Ts...>;

      /// What vector and revision was this iterator created from?
      const vec_t * v_ptr;
      int revision;

      // iterator_wrapper(const ITERATOR_T & _in)
      //   : ITERATOR_T(_in), v_ptr(nullptr), revision(0) { ; }
      iterator_wrapper(const ITERATOR_T & _in, const vec_t * _v)
        : ITERATOR_T(_in), v_ptr(_v), revision(_v->revision) { ; }
      iterator_wrapper(const this_t &) = default;
      iterator_wrapper(this_t &&) = default;
      ~iterator_wrapper() { ; }

      // Debug tools to make sure this iterator is okay.
      bool IsUsable() const {
        if (v_ptr == nullptr) return false;                      // Invalid vector
        if (v_ptr->revision == 0) return false;                  // Vector has been deleted!
        if (revision != v_ptr->revision) return false;           // Vector has changed memory!
        int pos = *this - v_ptr->begin();
        if (pos < 0 || pos >= (int) v_ptr->size()) return false; // Iterator out of range.
        return true;
      }

      // this_t & operator=(const ITERATOR_T & _in) { wrapped_t::operator=(_in); revision=0; }
      this_t & operator=(const this_t &) = default;
      this_t & operator=(this_t &&) = default;

      operator ITERATOR_T() { return *this; }
      operator const ITERATOR_T() const { return *this; }

      auto & operator*() {
        emp_assert(revision == v_ptr->revision);  // Ensure vector hasn't changed since making iterator.
        return wrapped_t::operator*();
      }
      const auto & operator*() const {
        emp_assert(revision == v_ptr->revision);  // Ensure vector hasn't changed since making iterator.
        return wrapped_t::operator*();
      }

      auto operator++() { return this_t(wrapped_t::operator++(), v_ptr); }
      auto operator++(int x) { return this_t(wrapped_t::operator++(x), v_ptr); }
      auto operator--() { return this_t(wrapped_t::operator--(), v_ptr); }
      auto operator--(int x) { return this_t(wrapped_t::operator--(x), v_ptr); }

      auto operator+(int in) { return this_t(wrapped_t::operator+(in), v_ptr); }
      auto operator-(int in) { return this_t(wrapped_t::operator-(in), v_ptr); }
      auto operator-(const this_t & in) { return ((wrapped_t) *this) - (wrapped_t) in; }
    };

    using iterator = iterator_wrapper< typename stdv_t::iterator >;
    using const_iterator = iterator_wrapper< typename stdv_t::const_iterator >;
    using reverse_iterator = iterator_wrapper< typename stdv_t::reverse_iterator >;
    using const_reverse_iterator = iterator_wrapper< typename stdv_t::const_reverse_iterator >;
    using value_type = T;
    using size_type = typename stdv_t::size_type;
    using reference = typename stdv_t::reference;
    using const_reference = typename stdv_t::const_reference;

    vector() : stdv_t(), revision(1) {};
    vector(const this_t & _in) : stdv_t(_in), revision(1) {};
    vector(size_t size) : stdv_t(size), revision(1) { emp_assert(size < MAX_SIZE, size); }
    vector(size_t size, const T & val) : stdv_t(size, val), revision(1) { emp_assert(size < MAX_SIZE, size); }
    vector(std::initializer_list<T> in_list) : stdv_t(in_list), revision(1) { ; }
    vector(const stdv_t & in) : stdv_t(in), revision(1) { ; }         // Emergency fallback conversion.
    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last), revision(1) { ; }
    ~vector() { revision = 0; } // Clear out revision when vector is deleted.

    iterator begin() noexcept { return iterator(stdv_t::begin(), this); }
    const_iterator begin() const noexcept { return const_iterator(stdv_t::begin(), this); }
    iterator end() noexcept { return iterator(stdv_t::end(), this); }
    const_iterator end() const noexcept { return const_iterator(stdv_t::end(), this); }

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
    // this_t & operator=(const this_t &) = default;

    T & operator[](size_t pos) {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    const T & operator[](size_t pos) const {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    T & back() { emp_assert(stdv_t::size() > 0); return stdv_t::back(); }
    const T & back() const { emp_assert(stdv_t::size() > 0); return stdv_t::back(); }
    T & front() { emp_assert(stdv_t::size() > 0); return stdv_t::front(); }
    const T & front() const { emp_assert(stdv_t::size() > 0); return stdv_t::front(); }

    void pop_back() {
      emp_assert(stdv_t::size() > 0, stdv_t::size());
      stdv_t::pop_back();
      revision++;           // Technically reducing size can cause memory reallocation, but less likely.
    }

    template <typename... PB_Ts>
    void push_back(PB_Ts &&... args) {
      stdv_t::push_back(std::forward<PB_Ts>(args)...);
      revision++;
    }

    template <typename... EB_Ts>
    void emplace_back(EB_Ts &&... args) {
      stdv_t::emplace_back(std::forward<EB_Ts>(args)...);
      revision++;
    }
  };

  /// Build a specialized debug wrapper for emp::vector<bool>
  template <typename... Ts>
  class vector<bool, Ts...> : public std::vector<bool,Ts...> {
  private:
    using this_t = emp::vector<bool,Ts...>;
    using stdv_t = std::vector<bool,Ts...>;

    /// Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2000000001;

  public:
    using iterator = typename stdv_t::iterator;
    using const_iterator = typename stdv_t::const_iterator;
    using value_type = bool;
    using size_type = typename stdv_t::size_type;
    using reference = typename stdv_t::reference;
    using const_reference = typename stdv_t::const_reference;

    vector() : stdv_t() {};
    vector(const this_t & _in) : stdv_t(_in) {};
    vector(size_t size) : stdv_t(size) { emp_assert(size < MAX_SIZE, size); }
    vector(size_t size, bool val) : stdv_t(size, val) { emp_assert(size < MAX_SIZE, size); }
    vector(std::initializer_list<bool> in_list) : stdv_t(in_list) { ; }
    vector(const stdv_t & in) : stdv_t(in) { ; }         // Emergency fallback conversion.
    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last){;}


    // operator stdv_t &() { return v; }
    // operator const stdv_t &() const { return v; }

    void resize(size_t new_size) { emp_assert(new_size < MAX_SIZE, new_size); stdv_t::resize(new_size); }
    void resize(size_t new_size, bool val) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size, val);
    }
    // this_t & operator=(const this_t &) = default;

    auto operator[](size_t pos) -> decltype(stdv_t::operator[](pos)) {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    bool operator[](size_t pos) const {
      emp_assert(pos < stdv_t::size(), pos, stdv_t::size());
      return stdv_t::operator[](pos);
    }

    auto & back() { emp_assert(stdv_t::size() > 0); return stdv_t::back(); }
    bool back() const { emp_assert(stdv_t::size() > 0); return stdv_t::back(); }
    auto & front() { emp_assert(stdv_t::size() > 0); return stdv_t::front(); }
    bool front() const { emp_assert(stdv_t::size() > 0); return stdv_t::front(); }

    void pop_back() {
      emp_assert(stdv_t::size() > 0, stdv_t::size());
      stdv_t::pop_back();
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

#endif

// namespace emp {
//   template<typename T, typename... Ts> struct TypeID< emp::vector<T,Ts...> > {
//     static std::string GetName() {
//       using simple_vt = emp::vector<T>;
//       using full_vt = emp::vector<T,Ts...>;
//       if (std::is_same<simple_vt,full_vt>::value) {
//         return "emp::vector<" + TypeID<T>::GetName() + ">";
//       }
//       return "emp::vector<" + TypeID<TypePack<T,Ts...>>::GetTypes() + ">";
//     }
//   };
// }

#endif
