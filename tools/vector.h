#ifndef EMP_VECTOR_H
#define EMP_VECTOR_H

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This class is a drop-in replacement for std::vector, but with bounds checking.
//  If EMP_NDEBUG is set then it reverts back to std::vector.
//

#include <vector>

#include "assert.h"


#ifdef EMP_NDEBUG

namespace emp {
  using vector = std::vector;
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
    ~vector() = default;

    uint32_t size() const { return v.size(); }
    void resize(int new_size) { emp_assert(size >= 0); v.resize(new_size); }

    T & operator[](int pos) {
      emp_assert(pos >= 0 && pos < (int) v.size());
      return v[pos];
    }

    const T & operator[](int pos) const {
      emp_assert(pos >= 0 && pos < (int) v.size());
      return v[pos];
    }

    template <typename T2>
    vector & push_back(T2 && in) { v.push_back(std::forward<T2>(in)); return *this; }

    auto begin() -> decltype(v.begin()) { return v.begin(); }
    auto end() -> decltype(v.end()) { return v.end(); }
  };

}

#endif



#endif
