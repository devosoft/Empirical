//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
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

// Seemlessly translate emp::vector to std::vector
namespace emp {
  template <typename... Ts> using vector = std::vector<Ts...>;
}


#else // #EMP_NDEBUG *not* set

namespace emp {

  // Build a debug wrapper emp::vector around std::vector.
  template <typename T, typename... Ts>
  class vector : public std::vector<T,Ts...> {
  private:
    using this_t = emp::vector<T,Ts...>;
    using stdv_t = std::vector<T,Ts...>;

    // Setup a threshold; if we try to make a vector bigger than MAX_SIZE, throw a warning.
    constexpr static const size_t MAX_SIZE = 2000000001;

  public:
    using iterator = typename stdv_t::iterator;
    using const_iterator = typename stdv_t::const_iterator;
    using value_type = T;
    using size_type = typename stdv_t::size_type;
    using reference = typename stdv_t::reference;
    using const_reference = typename stdv_t::const_reference;

    vector() : stdv_t() {};
    vector(const this_t & _in) : stdv_t(_in) {};
    vector(size_t size) : stdv_t(size) { emp_assert(size < MAX_SIZE, size); }
    vector(size_t size, const T & val) : stdv_t(size, val) { emp_assert(size < MAX_SIZE, size); }
    vector(std::initializer_list<T> in_list) : stdv_t(in_list) { ; }
    vector(const stdv_t & in) : stdv_t(in) { ; }         // Emergency fallback conversion.
    template <typename InputIt>
    vector(InputIt first, InputIt last) : stdv_t(first, last){;}


    // operator stdv_t &() { return v; }
    // operator const stdv_t &() const { return v; }

    void resize(size_t new_size) { emp_assert(new_size < MAX_SIZE, new_size); stdv_t::resize(new_size); }
    void resize(size_t new_size, const T & val) {
      emp_assert(new_size < MAX_SIZE, new_size);
      stdv_t::resize(new_size, val);
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
