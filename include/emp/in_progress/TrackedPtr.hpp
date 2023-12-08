/*
 *  This file is part of Empirical, https://github.com/devosoft/Empirical
 *  Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  date: 2022
*/
/**
 *  @file
 *  @brief Similar to Ptr, but memory is tracked and managed elsewhere, such as smart pointers.
 *  @note Status: ALPHA
 *
 */

#ifndef EMP_IN_PROGRESS_TRACKEDPTR_HPP_INCLUDE
#define EMP_IN_PROGRESS_TRACKEDPTR_HPP_INCLUDE

#include "Ptr.hpp"

namespace emp {

  class PtrManager {
  public:
    virtual void IncCount() = 0;
  };

  template <typename TYPE>
  class TrackedPtr : public Ptr<TYPE> {
  private:
    using BasePtr<TYPE>::ptr;

  public:
    using element_type = TYPE;

    /// Default constructor
    TrackedPtr() : Ptr<TYPE>(nullptr) {}

    /// Copy constructor
    TrackedPtr(const TrackedPtr<TYPE> & _in) : Ptr<TYPE>(_in) {}

    /// Construct from raw ptr
    template <typename T2> Ptr(T2 * in_ptr, bool=false) : BasePtr<TYPE>(in_ptr) {}

    /// Construct from array
    template <typename T2> Ptr(T2 * _ptr, size_t, bool) : BasePtr<TYPE>(_ptr) {}

    /// From compatible Ptr
    template <typename T2> Ptr(Ptr<T2> _in) : BasePtr<TYPE>(_in.Raw()) {}

    /// From nullptr
    Ptr(std::nullptr_t) : Ptr() {}

    /// Destructor
    ~Ptr() { ; }


}

#endif // #ifndef EMP_IN_PROGRESS_TRACKEDPTR_HPP_INCLUDE
