//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2016.
//  Released under the MIT Software license; see doc/LICENSE
//
//  TypeTracker is attached to other classes to easily convert them to their derived version
//  to facilitate type-specific operations.

#ifndef EMP_TYPE_TRACKER_H
#define EMP_TYPE_TRACKER_H

#include "assert.h"
#include "TypeSet.h"

namespace emp {

  // The base class of the types to be tracked should publically inherert from TypeTracker_Base
  struct TypeTracker_Base {
    virtual int GetTypeTrackerID() const noexcept {
      emp_assert(false); // Should never run GetTypeTrackerID from the base class!
      return -1;
    }
  };

  // The derived classes to be tracked should inherit from TypeTracker_Class<ID>
  // where ID is the position in the type list for TypeTracker.  Note that this value can
  // be obtained dyanmically at compile type by using TypeTracker<...>::GetID<TYPE>()
  template <int ID>
  struct TypeTracker_Class : public TypeTracker_Base {
    virtual int GetTypeTrackerID() const noexcept { return ID; }
  };

  template <typename T1, typename... Ts>
  struct TypeTracker : public TypeSet<T1,Ts...> {
  };

};

#endif
