//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeTracker is attached to other classes to easily convert them to their derived version
//  to facilitate type-specific operations.

#ifndef EMP_TYPE_TRACKER_H
#define EMP_TYPE_TRACKER_H

#include "assert.h"
#include "meta.h"
#include "array.h"

namespace emp {

  // The base class of the types to be tracked.
  struct TrackedType {
    virtual int GetTypeTrackerID() const noexcept {
      emp_assert(false); // Should never run GetTypeTrackerID from the base class!
      return -1;
    }
    virtual ~TrackedType() = default;
  };

  // The derived classes to be tracked should inherit from TypeTracker_Class<ID>
  // where ID is the position in the type list for TypeTracker.  Note that this value can
  // be obtained dyanmically at compile type by using TypeTracker<...>::GetID<TYPE>()
  template <typename REAL_T, int ID>
  struct TypeTracker_Class : public TrackedType {
    using real_t = REAL_T;
    REAL_T value;

    TypeTracker_Class(const REAL_T & in) : value(in) { ; }
    TypeTracker_Class(REAL_T && in) : value(std::forward<REAL_T>(in)) { ; }
    TypeTracker_Class(const TypeTracker_Class &) = default;
    TypeTracker_Class(TypeTracker_Class &&) = default;
    TypeTracker_Class & operator=(const TypeTracker_Class &) = default;
    TypeTracker_Class & operator=(TypeTracker_Class &&) = default;
    virtual int GetTypeTrackerID() const noexcept { return ID; }
  };

  template <typename... TYPES>
  struct TypeTracker {
    using this_t = TypeTracker<TYPES...>;
    template <typename REAL_T>
    using wrap_t = TypeTracker_Class< REAL_T, get_type_index<REAL_T,TYPES...>() >;

    constexpr static int GetNumTypes() { return sizeof...(TYPES)+1; }
    constexpr static int GetNumCombos() { return GetNumTypes() * GetNumTypes(); }

    emp::array< std::function<void(TrackedType*, TrackedType*)>, GetNumCombos() > redirects;

    TypeTracker() { ; }
    TypeTracker(const TypeTracker &) = default;
    TypeTracker(TypeTracker &&) = default;
    TypeTracker & operator=(const TypeTracker &) = default;
    TypeTracker & operator=(TypeTracker &&) = default;

    template <typename REAL_T>
    wrap_t<REAL_T> Wrap(REAL_T && val) { return wrap_t<REAL_T>(std::forward<REAL_T>(val)); }
    template <typename REAL_T>
    wrap_t<REAL_T> * New(REAL_T && val) { return new wrap_t<REAL_T>(std::forward<REAL_T>(val)); }

    template <typename T1, typename T2>
    this_t & AddFunction( std::function<void(T1,T2)> fun ) {
      constexpr int ID1 = get_type_index<T1,TYPES...>();
      constexpr int ID2 = get_type_index<T2,TYPES...>();
      constexpr int POS = ID1 * GetNumTypes() + ID2;
      redirects[POS] = [fun](TrackedType* b1, TrackedType* b2) {
        emp_assert(dynamic_cast<wrap_t<T1> *>(b1) != nullptr);
        emp_assert(dynamic_cast<wrap_t<T2> *>(b2) != nullptr);
        fun( ((wrap_t<T1> *) b1)->value, ((wrap_t<T2> *) b2)->value );
      };
      return *this;
    }

    template <typename T1, typename T2>
    this_t & AddFunction( void (*fun)(T1,T2) ) {
      return AddFunction( std::function<void(T1,T2)>(fun) );
    }

    void RunFunction( TrackedType * b1, TrackedType * b2 ) {
      const int id1 = b1->GetTypeTrackerID();
      const int id2 = b2->GetTypeTrackerID();
      const int pos = id1 * GetNumTypes() + id2;
      if (redirects[pos]) redirects[pos](b1,b2);  // If a redirect exists, use it!
    }
  };

}

#endif
