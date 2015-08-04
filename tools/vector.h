#ifndef EMP_VECTOR_H
#define EMP_VECTOR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class is a drop-in replacement for std::vector, but with bounds checking.
//  If EMP_NDEBUG is set then it reverts back to std::vector.
//

#include <initializer_list>
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
    vector() = default;
    vector(const emp::vector<T> &) = default;
    vector(int size) : v(size) { emp_assert(size >= 0); }
    vector(int size, const T & val) : v(size, val) { emp_assert(size >= 0); }
    vector(std::initializer_list<T> in_list) : v(in_list) { ; }
    vector(const std::vector<T> & in) : v(in) { ; }         // Emergency fallback conversion.
    ~vector() = default;

    uint32_t size() const { return v.size(); }
    void resize(int new_size) { emp_assert(new_size >= 0); v.resize(new_size); }

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
      emp_assert(pos >= 0 && pos < (int) v.size());
      return v[pos];
    }

    template <typename T2>
    vector & push_back(T2 && in) { v.push_back(std::forward<T2>(in)); return *this; }

    template <typename... T2>
    vector & emplace_back(T2 &&... in) { v.push_back(std::forward<T2>(in)...); return *this; }

    auto begin() -> decltype(v.begin()) { return v.begin(); }
    auto end() -> decltype(v.end()) { return v.end(); }
    auto begin() const -> const decltype(v.begin()) { return v.begin(); }
    auto end() const -> const decltype(v.end()) { return v.end(); }

    T & back() { return v.back(); }
    const T & back() const { return v.back(); }
  };

}

#endif



#endif
