/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file TypeTracker.hpp
 *  @brief Track class types abstractly to dynamically call correct function overloads.
 *  @note Status: BETA
 *
 *  TypeTracker is a templated class that must be declared with all of the types that can
 *  possibly be tracked.  For example:
 *
 *      emp::TypeTracker<int, std::string, double> tt;
 *
 *  ...would create a TypeTracker that can manage the three types listed and convert back.
 *
 *  @todo Should use std::is_convertible<X,Y>::value to determine if casting on base type is allowed.
 *  @todo Functions should be able to have fixed type values mixed in.
 */

#ifndef EMP_TOOLS_TYPETRACKER_HPP_INCLUDE
#define EMP_TOOLS_TYPETRACKER_HPP_INCLUDE

#include <unordered_map>

#include "../base/array.hpp"
#include "../base/assert.hpp"
#include "../base/Ptr.hpp"
#include "../datastructs/map_utils.hpp"
#include "../functional/GenericFunction.hpp"
#include "../math/math.hpp"
#include "../meta/meta.hpp"

namespace emp {

  /// The proxy base class of any type to be tracked.
  struct TrackedInfo_Base {
    virtual size_t GetTypeID() const noexcept = 0;
    virtual ~TrackedInfo_Base () {;}
    virtual emp::Ptr<TrackedInfo_Base> Clone() const = 0;
  };

  /// The actual TrackedVar object that manages a Ptr to the value.
  struct TrackedVar {
    emp::Ptr<TrackedInfo_Base> ptr;

    // Note: This is the primary constructor for new TrackedVar and non-null values should be
    //       built only in TypeTracker; since that is a template, it can't be a friend.
    TrackedVar(emp::Ptr<TrackedInfo_Base> _ptr) : ptr(_ptr) { ; }

    /// Copy constructor; use judiciously since it copies the contents!
    TrackedVar(const TrackedVar & _in) : ptr(nullptr) { if (_in.ptr) ptr = _in.ptr->Clone(); }

    /// Move constructor takes control of the pointer.
    TrackedVar(TrackedVar && _in) : ptr(_in.ptr) { _in.ptr = nullptr; }

    /// Cleanup ptr on destruct.
    ~TrackedVar() { if (ptr) ptr.Delete(); }

    /// Move assignment hands over control of the pointer.
    TrackedVar & operator=(const TrackedVar & _in) {
      if (ptr) ptr.Delete();
      ptr = _in.ptr->Clone();
      return *this;
    }

    /// Move assignment hands over control of the pointer.
    TrackedVar & operator=(TrackedVar && _in) {
      if (ptr) ptr.Delete();
      ptr = _in.ptr;
      _in.ptr = nullptr;
      return *this;
    }

    size_t GetTypeID() const noexcept { return ptr->GetTypeID(); }
  };

  /// TrackedInfo_Value store both the real type and an ID for it (to be identified from
  /// the base class for each conversion back.)
  template <typename REAL_T, size_t ID>
  struct TrackedInfo_Value : public TrackedInfo_Base {
    using real_t = REAL_T;
    using this_t = TrackedInfo_Value<REAL_T,ID>;
    REAL_T value;

    TrackedInfo_Value(const REAL_T & in) : value(in) { ; }
    TrackedInfo_Value(REAL_T && in) : value(std::forward<REAL_T>(in)) { ; }
    TrackedInfo_Value(const TrackedInfo_Value &) = default;
    TrackedInfo_Value(TrackedInfo_Value &&) = default;

    TrackedInfo_Value & operator=(const TrackedInfo_Value &) = default;
    TrackedInfo_Value & operator=(TrackedInfo_Value &&) = default;

    size_t GetTypeID() const noexcept override { return ID; }

    /// Build a copy of this TrackedInfo_Value; recipient is in charge of deletion.
    emp::Ptr<TrackedInfo_Base> Clone() const override {
      return emp::NewPtr<this_t>(value);
    }
  };

  /// Dynamic functions that are indexed by parameter types; calls lookup the correct function
  /// to forward arguments into.
  template <typename... TYPES>
  class TypeTracker {
  protected:
    /// fun_map is a hash table that maps a set of inputs to the appropriate function.
    std::unordered_map<size_t, Ptr<emp::GenericFunction>> fun_map;

  public:
    // Constructors!
    TypeTracker() = default;
    TypeTracker(const TypeTracker &) = default;
    TypeTracker(TypeTracker &&) = default;
    TypeTracker & operator=(const TypeTracker &) = default;
    TypeTracker & operator=(TypeTracker &&) = default;

    // Destructor!
    ~TypeTracker() {
      for (auto x : fun_map) x.second.Delete();  // Clear out Functions.
    }

    using this_t = TypeTracker<TYPES...>;
    template <typename REAL_T>
    using wrap_t = TrackedInfo_Value< REAL_T, get_type_index<REAL_T,TYPES...>() >;

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
    constexpr static size_t GetID() {
      static_assert(get_type_index<T,TYPES...>() != -1, "Can only get IDs for pre-specified types.");
      return (size_t) get_type_index<T,TYPES...>();
    }

    /// Each set of types should have an ID unique within that number of types.
    template <typename T1, typename T2, typename... Ts>
    constexpr static size_t GetID() { return GetID<T1>() + GetID<T2,Ts...>() * GetNumTypes(); }

    /// A ComboID should be unique *across* all size combinations.
    template <typename... Ts>
    constexpr static size_t GetComboID() {
      return GetCumCombos(sizeof...(Ts)-1) + GetID<Ts...>();
    }

    /// A Tracked ID is simply the unique ID of the type being tracked.
    static size_t GetTrackedID(const TrackedVar & tt) { return tt.GetTypeID(); }

    /// Or set of types being tracked...
    template <typename... Ts>
    static size_t GetTrackedID(const TrackedVar & tt1, const TrackedVar & tt2, const Ts &... ARGS) {
      return tt1.GetTypeID() + GetTrackedID(tt2, ARGS...) * GetNumTypes();
    }

    /// A tracked COMBO ID, is an ID for this combination of types, unique among all possible type
    /// combinations.  Consistent with GetComboID with the same underlying types.
    template <typename... Ts>
    constexpr static size_t GetTrackedComboID(const Ts &... ARGS) {
      return GetCumCombos(sizeof...(Ts)-1) + GetTrackedID(ARGS...);
    }

    /// Convert an input value into a TrackedInfo_Value maintaining the value (universal version)
    template <typename REAL_T>
    static TrackedVar Convert(const REAL_T & val) {
      emp_assert((has_type<REAL_T,TYPES...>()));    // Make sure we're wrapping a legal type.
      using decay_t = std::decay_t<REAL_T>;
      return TrackedVar( NewPtr<wrap_t<decay_t>>(val) );
    }

    /// Test if the tracked type is TEST_T
    template <typename TEST_T>
    static bool IsType( TrackedVar & tt ) {
      return tt.GetTypeID() == get_type_index<TEST_T,TYPES...>();
    }

    /// Convert the tracked type back to REAL_T.  Assert that this is type safe!
    template <typename REAL_T>
    static REAL_T ToType( TrackedVar & tt ) {
      emp_assert(IsType<REAL_T>(tt));
      return (tt.ptr.Cast<wrap_t<REAL_T>>() )->value;
    }

    /// Cast the tracked type to OUT_T.  Try to do so even if NOT original type!
    template <typename OUT_T>
    static OUT_T Cast( TrackedVar & tt ) { return tt.ptr.Cast<wrap_t<OUT_T>>()->value; }

    /// var_decoy converts any variable into a TrackedVar (used to have correct number of vars)
    template <typename T> using var_decoy = TrackedVar;

    /// Add a new std::function that this TypeTracker should call if the appropriate types are
    /// passed in.
    template <typename... Ts>
    this_t & AddFunction( std::function<void(Ts...)> fun ) {
      constexpr size_t ID = GetComboID<Ts...>();

      // We need to ensure there are the same number of TrackedVar parameters in the wrapped
      // function as there were typed parameters in the original.  To accomplish this task, we
      // will expand the original type pack, but use decoys to convert to TrackedVar.

      auto fun_wrap = [fun](var_decoy<Ts> &... args) {
        // Ensure all types can be cast appropriately
        emp_assert( AllTrue( args.ptr. template DynamicCast<wrap_t<Ts>>()... ) );

        // Now run the function with the correct type conversions
        fun( (args.ptr. template Cast<wrap_t<Ts>>())->value... );
      };

      fun_map[ID] = emp::NewPtr<Function<void(var_decoy<Ts> &...)>>(fun_wrap);

      return *this;
    }

    /// Add a new function pointer that this TypeTracker should call if the appropriate types are
    /// passed in.
    template <typename... Ts>
    this_t & AddFunction( void (*fun)(Ts...) ) {
      return AddFunction( std::function<void(Ts...)>(fun) );
    }

    /// Add a new lambda function that this TypeTracker should call if the appropriate types are
    /// passed in.
    template <typename LAMBDA_T>
    this_t & AddFunction( const LAMBDA_T & fun ) {
      return AddFunction( to_function(fun) );
    }

    /// Run the appropriate function based on the argument types received.
    template <typename... Ts>
    void RunFunction( Ts &&... args ) {                 // args must all be TrackedVar pointers!
      const size_t pos = GetTrackedComboID(args...);
      if (Has(fun_map, pos)) {  // If a redirect exists, use it!
        Ptr<GenericFunction> gfun = fun_map[pos];
        gfun->Call<void, var_decoy<Ts> &...>(args...);
      }
    }

    /// Call TypeTracker as a function (refers call to RunFunction)
    template <typename... Ts>
    void operator()(Ts &&... args) { RunFunction(args...); }
  };

}

#endif // #ifndef EMP_TOOLS_TYPETRACKER_HPP_INCLUDE
