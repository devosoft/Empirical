/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2017
 *
 *  @file TypeTracker.h
 *  @brief Track class types abstractly to dynamically call correct function overloads.
 *  @note Status: BETA
 *
 *  @todo Should use std::is_convertible<X,Y>::value to determine if casting on base type is allowed.
 *  @todo Functions should be able to have fixed type values mixed in.
 */

#ifndef EMP_TYPE_TRACKER_H
#define EMP_TYPE_TRACKER_H

#include <unordered_map>

#include "../base/array.h"
#include "../base/assert.h"
#include "../meta/meta.h"

#include "functions.h"
#include "GenericFunction.h"
#include "map_utils.h"

namespace emp {

  /// The base class of any type to be tracked.
  struct TrackedType {
    virtual size_t GetTypeTrackerID() const noexcept = 0;
    virtual ~TrackedType() {;}
  };

  /// The derived classes to be tracked should inherit from TypeTracker_Class<ID>
  /// where ID is the position in the type list for TypeTracker.  Note: this value can
  /// be obtained dyanmically at compile type by using TypeTracker<...>::GetID<TYPE>()
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

  /// Dynamic functions that are indexed by parameter types; calls lookup the correct function
  /// to forward arguments into.
  template <typename... TYPES>
  struct TypeTracker {
    using this_t = TypeTracker<TYPES...>;
    template <typename REAL_T>
    using wrap_t = TypeTracker_Class< REAL_T, get_type_index<REAL_T,TYPES...>() >;

    /// How many types are we working with?
    constexpr static size_t GetNumTypes() { return sizeof...(TYPES); }

    /// How many combinations of V types are there?
    constexpr static size_t GetNumCombos(size_t vals=2) {
      size_t result = 1;
      for (size_t v = 0; v < vals; v++) result *= GetNumTypes();
      return result;
    }

    /// How many combinations are the of the given number of types OR FEWER?
    constexpr static size_t GetCumCombos(size_t vals=2) {
      size_t cur_result = 1;
      size_t cum_result = 1;
      for (size_t v = 0; v < vals; v++) {
        cur_result *= GetNumTypes();
        cum_result += cur_result;
      }
      return cum_result;
    }

    /// Each type should have a unique ID.
    template <typename T>
    constexpr static size_t GetID() { return get_type_index<T,TYPES...>(); }

    /// Each set of types should have an ID unique within that number of types.
    template <typename T1, typename T2, typename... Ts>
    constexpr static size_t GetID() { return GetID<T1>() + GetID<T2,Ts...>() * GetNumTypes(); }

    /// A ComboID should be unique *across* all size combinations.
    template <typename... Ts>
    constexpr static size_t GetComboID() {
      return GetCumCombos(sizeof...(Ts)-1) + GetID<Ts...>();
    }

    /// A Tracked ID is simply the unique ID of the type being tracked.
    static size_t GetTrackedID(const TrackedType & tt) { return tt.GetTypeTrackerID(); }
    template <typename... Ts>

    /// Or set of types being tracked...
    static size_t GetTrackedID(const TrackedType & tt1, const TrackedType & tt2, const Ts &... ARGS) {
      return tt1.GetTypeTrackerID() + GetTrackedID(tt2, ARGS...) * GetNumTypes();
    }

    /// We should also about able to use a pointer to access tracked IDs
    static size_t GetTrackedID(TrackedType * tt) { return tt->GetTypeTrackerID(); }

    /// A set of pointers to access tracked IDs
    template <typename... Ts>
    static size_t GetTrackedID(TrackedType * tt1, TrackedType * tt2, Ts *... ARGS) {
      return tt1->GetTypeTrackerID() + GetTrackedID(tt2, ARGS...) * GetNumTypes();
    }

    /// A tracked COMBO ID, is an ID for this combination of types, unique among all possible type
    /// combinations.  Consistent with GetComboID with the same underlying types.
    template <typename... Ts>
    constexpr static size_t GetTrackedComboID(Ts... ARGS) {
      return GetCumCombos(sizeof...(Ts)-1) + GetTrackedID(ARGS...);
    }

    /// fun_map is a hash table that maps a set of inputs to the appropriate function.
    std::unordered_map<size_t, emp::GenericFunction *> fun_map;

    // Constructors!
    TypeTracker() : fun_map() { ; }
    TypeTracker(const TypeTracker &) = default;
    TypeTracker(TypeTracker &&) = default;
    TypeTracker & operator=(const TypeTracker &) = default;
    TypeTracker & operator=(TypeTracker &&) = default;

    // Destructor!
    ~TypeTracker() {
      for (auto x : fun_map) delete x.second;  // Clear out Functions.
    }

    /// Convert an input value into a TypeTracker_Class maintaining the value (universal version)
    template <typename REAL_T> wrap_t<REAL_T> Wrap(REAL_T && val) {
      emp_assert((has_type<REAL_T,TYPES...>()));    // Make sure we're wrapping a legal type.
      return wrap_t<REAL_T>(std::forward<REAL_T>(val));
    }

    /// Create an input value in a TypeTracker_Class maintaining the value (reference version)
    template <typename REAL_T> wrap_t<REAL_T> * New(REAL_T & val) {
      emp_assert((has_type<REAL_T, TYPES...>()));   // Make sure we're wrapping a legal type.
      return new wrap_t<REAL_T>(val);
    }

    /// Create an input value in a TypeTracker_Class maintaining the value (move version)
    template <typename REAL_T> wrap_t<REAL_T> * New(REAL_T && val) {
      emp_assert((has_type<REAL_T, TYPES...>()));   // Make sure we're wrapping a legal type.
      return new wrap_t<REAL_T>(std::forward<REAL_T>(val));
    }

    /// Test if the tracked type is TEST_T
    template <typename TEST_T>
    bool IsType( TrackedType & tt ) {
      return tt.GetTypeTrackerID() == get_type_index<TEST_T,TYPES...>();
    }

    /// Test if the tracked type points to TEST_T
    template <typename TEST_T>
    bool IsType( TrackedType * tt ) { return IsType(*tt); }

    /// Convert the tracked type back to REAL_T.  Assert that this is type safe!
    template <typename REAL_T>
    REAL_T ToType( TrackedType & tt ) {
      emp_assert(IsType<REAL_T>(tt));
      return ((wrap_t<REAL_T> *) &tt)->value;
    }

    /// Convert the tracked type pointer back to REAL_T.  Assert that this is type safe!
    template <typename REAL_T>
    REAL_T ToType( TrackedType * tt ) { return ToType(*tt); }

    /// Cast the tracked type to OUT_T.  Try to do so even if NOT original type!
    template <typename OUT_T>
    OUT_T Cast( TrackedType & tt ) { return ((wrap_t<OUT_T> *) &tt)->value; }

    /// Cast the tracked type pointer to OUT_T.  Try to do so even if NOT original type!
    template <typename OUT_T>
    OUT_T Cast( TrackedType * tt ) { return Cast(*tt); }

    /// Add a new std::function that this TypeTracker should call if the appropriate types are
    /// passed in.
    template <typename... Ts>
    this_t & AddFunction( std::function<void(Ts...)> fun ) {
      constexpr size_t ID = GetComboID<Ts...>();

      // We need to ensure there are the same number of TrackedType parameters in the wrapped
      // function as there were typed parameters in the original.  To accomplish this task, we
      // will expand the original type pack, but use decoys to convert to TrackedType.

      auto fun_wrap = [fun](emp::type_decoy<TrackedType *,Ts>... args) {
        // Ensure all types can be cast appropriately
        emp_assert( AllTrue( dynamic_cast<wrap_t<Ts> *>(args)... ) );

        // Now run the function with the correct type conversions
        fun( ((wrap_t<Ts> *) args)->value... );
      };

      fun_map[ID] = new Function<void(emp::type_decoy<TrackedType *,Ts>...)>(fun_wrap);

      return *this;
    }

    /// Add a new function pointer that this TypeTracker should call if the appropriate types are
    /// passed in.
    template <typename... Ts>
    this_t & AddFunction( void (*fun)(Ts...) ) {
      return AddFunction( std::function<void(Ts...)>(fun) );
    }

    /// Run the appropriate function based on the argument types received.
    template <typename... Ts>
    void RunFunction( Ts... args ) {                 // args must all be TrackedType pointers!
      const size_t pos = GetTrackedComboID(args...);
      if (Has(fun_map, pos)) {  // If a redirect exists, use it!
        GenericFunction * gfun = fun_map[pos];
        gfun->Call<void, emp::type_decoy<TrackedType *,Ts>...>(((emp::type_decoy<TrackedType *,Ts>) args)...);
      }
    }

    /// Call TypeTracker as a function (refers call to RunFunction)
    template <typename... Ts>
    void operator()(Ts... args) { RunFunction(args...); }
  };

}

#endif
