//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016-2017.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  TypeTracker is attached to other classes to easily convert them to their derived version
//  to facilitate type-specific operations.
//  Status: BETA
//
//
//  Developer notes:
//  * Should use std::is_convertible<X,Y>::value to determine if casting on base type is allowed.
//  * AddFunction and RunFunction should be able to take any number of args.
//  * Functions should be able to have fixed type values mixed in.
//  * RunFunction should be able to have the function passed in at call time.

#ifndef EMP_TYPE_TRACKER_H
#define EMP_TYPE_TRACKER_H

#include "../base/array.h"
#include "../base/assert.h"
#include "../meta/meta.h"

namespace emp {

  // The base class of the types to be tracked.
  struct TrackedType {
    virtual size_t GetTypeTrackerID() const noexcept = 0;
    virtual ~TrackedType() {;}
  };

  // The derived classes to be tracked should inherit from TypeTracker_Class<ID>
  // where ID is the position in the type list for TypeTracker.  Note that this value can
  // be obtained dyanmically at compile type by using TypeTracker<...>::GetID<TYPE>()
  template <typename REAL_T, size_t ID>
  struct TypeTracker_Class : public TrackedType {
    using real_t = REAL_T;
    REAL_T value;

    TypeTracker_Class(const REAL_T & in) : value(in) { ; }
    TypeTracker_Class(REAL_T && in) : value(std::forward<REAL_T>(in)) { ; }
    TypeTracker_Class(const TypeTracker_Class &) = default;
    TypeTracker_Class(TypeTracker_Class &&) = default;
    TypeTracker_Class & operator=(const TypeTracker_Class &) = default;
    TypeTracker_Class & operator=(TypeTracker_Class &&) = default;
    virtual size_t GetTypeTrackerID() const noexcept { return ID; }
  };

  template <typename... TYPES>
  struct TypeTracker {
    using this_t = TypeTracker<TYPES...>;
    template <typename REAL_T>
    using wrap_t = TypeTracker_Class< REAL_T, get_type_index<REAL_T,TYPES...>() >;


    // How many types are we working with?
    constexpr static size_t GetNumTypes() { return sizeof...(TYPES); }

    // How many combinations of V types are there?
    constexpr static size_t GetNumCombos(size_t vals=2) {
      size_t result = 1;
      for (size_t v = 0; v < vals; v++) result *= GetNumTypes();
      return result;
    }

    // How many combinations are the of the given number of types OR FEWER?
    constexpr static size_t GetCumCombos(size_t vals=2) {
      size_t cur_result = 1;
      size_t cum_result = 1;
      for (size_t v = 0; v < vals; v++) {
        cur_result *= GetNumTypes();
        cum_result += cur_result;
      }
      return cum_result;
    }

    // Each type should have a unique ID.
    template <typename T>
    constexpr static size_t GetID() { return get_type_index<T,TYPES...>(); }

    // Each set of types should have an ID unique within that number of types.
    template <typename T1, typename T2, typename... Ts>
    constexpr static size_t GetID() { return GetID<T1>() + GetID<T2,Ts...>() * GetNumTypes(); }

    // A ComboID should be unique *across* all size combinations.
    template <typename... Ts>
    constexpr static size_t GetComboID() {
      return GetCumCombos(sizeof...(Ts)-1) + GetID<Ts...>();
    }

    // A Tracked ID is simply the unique ID of the type being tracked.
    static size_t GetTrackedID(const TrackedType & tt) { return tt.GetTypeTrackerID(); }
    template <typename... Ts>

    // Or set of types being tracked...
    static size_t GetTrackedID(const TrackedType & tt1, const TrackedType & tt2, const Ts &... ARGS) {
      return tt1.GetTypeTrackerID() + GetTrackedID(tt2, ARGS...) * GetNumTypes();
    }

    // We should also about able to use a pointer to access tracked IDs
    static size_t GetTrackedID(TrackedType * tt) { return tt->GetTypeTrackerID(); }
    template <typename... Ts>
    static size_t GetTrackedID(TrackedType * tt1, TrackedType * tt2, Ts *... ARGS) {
      return tt1->GetTypeTrackerID() + GetTrackedID(tt2, ARGS...) * GetNumTypes();
    }

    // A tracked COMBO ID, is an ID for this combination of types, unique among all possible type
    // combinations.  Consistent with GetComboID with the same underlying types.
    template <typename... Ts>
    constexpr static size_t GetTrackedComboID(Ts... ARGS) {
      return GetCumCombos(sizeof...(Ts)-1) + GetTrackedID(ARGS...);
    }

    // The redirects points a function call to the appropriate function, giving the input types.
    emp::array< std::function<void(TrackedType*, TrackedType*)>, GetNumCombos(2) > redirects;

    // Constructors!
    TypeTracker() { ; }
    TypeTracker(const TypeTracker &) = default;
    TypeTracker(TypeTracker &&) = default;
    TypeTracker & operator=(const TypeTracker &) = default;
    TypeTracker & operator=(TypeTracker &&) = default;

    template <typename REAL_T> wrap_t<REAL_T> Wrap(REAL_T && val) {
      emp_assert((has_type<REAL_T,TYPES...>()));    // Make sure we're wrapping a legal type.
      return wrap_t<REAL_T>(std::forward<REAL_T>(val));
    }
    template <typename REAL_T> wrap_t<REAL_T> * New(REAL_T & val) {
      emp_assert((has_type<REAL_T, TYPES...>()));   // Make sure we're wrapping a legal type.
      return new wrap_t<REAL_T>(val);
    }
    template <typename REAL_T> wrap_t<REAL_T> * New(REAL_T && val) {
      emp_assert((has_type<REAL_T, TYPES...>()));   // Make sure we're wrapping a legal type.
      return new wrap_t<REAL_T>(std::forward<REAL_T>(val));
    }

    // Test if the tracked type is TEST_T
    template <typename TEST_T>
    bool IsType( TrackedType & tt ) {
      return tt.GetTypeTrackerID() == get_type_index<TEST_T,TYPES...>();
    }
    template <typename TEST_T>
    bool IsType( TrackedType * tt ) { return IsType(*tt); }

    // Convert the tracked type back to REAL_T.  Assert that this is type safe!
    template <typename REAL_T>
    REAL_T ToType( TrackedType & tt ) {
      emp_assert(IsType<REAL_T>(tt));
      return ((wrap_t<REAL_T> *) &tt)->value;
    }
    template <typename REAL_T>
    REAL_T ToType( TrackedType * tt ) { return ToType(*tt); }

    // Cast the tracked type to OUT_T.  Try to do so even if NOT original type!
    template <typename OUT_T>
    OUT_T Cast( TrackedType & tt ) { return ((wrap_t<OUT_T> *) &tt)->value; }
    template <typename OUT_T>
    OUT_T Cast( TrackedType * tt ) { return Cast(*tt); }

    template <typename T1, typename T2>
    this_t & AddFunction( std::function<void(T1,T2)> fun ) {
      constexpr size_t POS = GetID<T1,T2>();
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
      const size_t pos = GetTrackedID(b1, b2);
      if (redirects[pos]) redirects[pos](b1,b2);  // If a redirect exists, use it!
    }
    void operator()( TrackedType * b1, TrackedType * b2 ) { RunFunction(b1,b2); }
  };

}

#endif
