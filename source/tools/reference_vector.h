/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  reference_vector.h
 *  @brief A version of vector that holds only references to objects.  Be careful!
 *  @note Status: ALPHA
 **/

#ifndef EMP_REFERENCE_VECTOR_H
#define EMP_REFERENCE_VECTOR_H

#include "../base/Ptr.h"
#include "../base/vector.h"

namespace emp {

  template <typename T>
  class reference_vector {
  private:
    using this_t = reference_vector<T>;
    emp::vector< emp::Ptr<T> > vals;

  public:
    reference_vector() = default;
    reference_vector(const reference_vector &) = default;
    reference_vector(reference_vector &&) = default;
    ~reference_vector() { }

    this_t & operator=(const reference_vector &) = default;
    this_t & operator=(reference_vector &&) = default;

    size_t size() const { return vals.size(); }
    void resize(size_t new_size) {
      emp_assert(new_size <= vals.size(),
                 "A reference_vector can only be resized smaller; grow using push_back()",
                 new_size, size());
      vals.resize(new_size);
    }

    T & operator[](size_t id) { return *(vals[id]); }
    const T & operator[](size_t id) const { return *(vals[id]); }

    void push_back(T & val) { vals.push_back(&val); }
  };

}

#endif