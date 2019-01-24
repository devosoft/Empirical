/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ActivationHandle.h
 *  @brief A generic handle that, depending on its current activation state, will dereference as an empty std::optional or a full std::optional.
 *  @note Status: BETA
 */


#ifndef EMP_ACTIVATION_HANDLE_H
#define EMP_ACTIVATION_HANDLE_H

#include "base/Ptr.h"

namespace emp {


  template<typename T>
  class ActivationHandle {

    private:
      bool active;
      T obj;

    public:

      /// construct the handle and its object,
      /// forwarding arguments to make the object in place
      /// inside a std::optional
      template<typename... Args>
      ActivationHandle(
        bool active_,
        Args&&... args
      ) : active(active_)
        , obj(std::forward<Args>(args)...)
      { ; }
      /// construct the handle and its object,
      /// forwarding arguments to make the object in place
      /// inside a std::optional
      template<typename... Args>
      ActivationHandle(
        Args&&... args
      ) : ActivationHandle(true, std::forward<Args>(args)...)
      { ; }

      /// override the dereference operator, returning the obj or nullopt
      /// depending on activation state
      emp::Ptr<T> operator *() { return active ? &obj : nullptr; }

      /// toggle the activation state
      void Toggle() { active = !active; }

      /// set the activation state
      void Set(bool s) { active = s; }

  };

}

#endif
