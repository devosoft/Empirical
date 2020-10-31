/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  reference_vector.hpp
 *  @brief A version of vector that holds only references to objects.  Be careful!
 *  @note Status: ALPHA
 **/

#ifndef EMP_REFERENCE_VECTOR_H
#define EMP_REFERENCE_VECTOR_H

#include "../base/Ptr.hpp"
#include "../base/vector.hpp"

namespace emp {

  template <typename T>
  class reference_vector {
  private:
    using this_t = reference_vector<T>;
    using vec_t = emp::vector< emp::Ptr<T> >;
    vec_t vals;

    class Iterator {
     ///  @todo Add a const interator, and probably a reverse iterator.
    private:
      typename vec_t::iterator it;

    public:
      Iterator(this_t & v, size_t pos=0) : it(v.vals.begin() + pos) { ; }
      Iterator(const typename vec_t::iterator & in_it) : it(in_it) { ; }
      Iterator(const Iterator &) = default;
      Iterator & operator=(const Iterator &) = default;

      Iterator & operator++() { ++it; return *this; }
      Iterator & operator--() { --it; return *this; }

      /// Iterator comparisons
      bool operator==(const Iterator& in) const { return it == in.it; }
      bool operator!=(const Iterator& in) const { return it != in.it; }
      bool operator< (const Iterator& in) const { return it <  in.it; }
      bool operator<=(const Iterator& in) const { return it <= in.it; }
      bool operator> (const Iterator& in) const { return it >  in.it; }
      bool operator>=(const Iterator& in) const { return it >= in.it; }

      /// Return a reference to the element pointed to by this iterator; may advance iterator.
      T & operator*() { return *(*it); }

      /// Return a const reference to the element pointed to by this iterator.
      const T & operator*() const { return *(*it); }

      /// Is this iterator pointing to a valid position in the vector?
      operator bool() const { return (bool) it; }

      /// Return an iterator pointing to the first position in the vector.
      Iterator begin() { return Iterator(it.begin()); }

      /// Return a const iterator pointing to the first position in the vector.
      const Iterator begin() const { return Iterator(it.begin()); }

      /// Return an iterator pointing to just past the end of the vector.
      Iterator end() { return Iterator(it.end()); }

      /// Return a const iterator pointing to just past the end of the vector.
      const Iterator end() const { return Iterator(it.end()); }
    };

    using iterator_t = Iterator;
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

    // Iterators
    Iterator begin() { return Iterator(vals.begin()); }
    const Iterator begin() const { return Iterator(vals.begin()); }
    Iterator end() { return Iterator(vals.end()); }
    const Iterator end() const { return Iterator(vals.end()); }
  };

}

#endif