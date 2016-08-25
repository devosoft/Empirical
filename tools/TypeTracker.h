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
#include "vector.h"

namespace emp {

  // The base class of the types to be tracked; derived from BASE which can include any common info.
  template <typename BASE>
  struct TypeTracker_Base : public BASE {
    virtual int GetTypeTrackerID() const noexcept {
      emp_assert(false); // Should never run GetTypeTrackerID from the base class!
      return -1;
    }
  };

  // The derived classes to be tracked should inherit from TypeTracker_Class<ID>
  // where ID is the position in the type list for TypeTracker.  Note that this value can
  // be obtained dyanmically at compile type by using TypeTracker<...>::GetID<TYPE>()
  template <typename BASE, typename OWNER, int ID>
  struct TypeTracker_Class : public TypeTracker_Base<BASE> {
    using owner_t = OWNER;
    OWNER * owner;
    virtual int GetTypeTrackerID() const noexcept { return ID; }
  };

  template <typename BASE, typename FIRST_T, typename... OTHER_Ts>
  struct TypeTracker {
    using this_t = TypeTracker<BASE, FIRST_T, OTHER_Ts...>;
    using base_t = TypeTracker_Base<BASE>;
    template <typename OWNER>
    using wrap = TypeTracker_Class< BASE, OWNER, get_type_index<OWNER,FIRST_T,OTHER_Ts...>() >;
    constexpr static int GetNumTypes() { return sizeof...(OTHER_Ts)+1; }
    constexpr static int GetNumCombos() { return GetNumTypes() * GetNumTypes(); }

    emp::vector< std::function<void(base_t*, base_t*)> > redirects;

    TypeTracker() : redirects(GetNumCombos()) { ; }
    TypeTracker(const TypeTracker &) = default;
    TypeTracker & operator=(const TypeTracker &) = default;

    template <typename T1, typename T2>
    this_t & AddFunction( std::function<void(T1*,T2*)> fun ) {
      constexpr int ID1 = get_type_index<T1,FIRST_T,OTHER_Ts...>();
      constexpr int ID2 = get_type_index<T2,FIRST_T,OTHER_Ts...>();
      constexpr int POS = ID1 * GetNumTypes() + ID2;
      redirects[POS] = [fun](base_t* b1, base_t* b2) {
        fun(((wrap<T1> *) b1)->owner, ((wrap<T2> *) b2)->owner);
      };
      return *this;
    }
  };

}

#endif
