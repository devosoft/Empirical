#ifndef EMP_REFLECTION_H
#define EMP_REFLECTION_H

////////////////////////////////////////////////////////////////////////////////////////
//
//  This class provides utilities to help determine details about classes.
//

#include "functions.h"

// The macro below creates a struct capable of detecting if another class possesses a
// specific member.  From: https://en.wikibooks.org/wiki/More_C++_Idioms/Member_Detector

#define EMP_CREATE_MEMBER_DETECTOR(MEMBER_NAME)                         \
  template<typename T>                                                  \
  class EMP_Detect_ ## MEMBER_NAME {                                    \
  private:                                                              \
    struct Fallback { int MEMBER_NAME; };                               \
    struct Derived : T, Fallback { };                                   \
                                                                        \
    template<typename U, U> struct Check;                               \
                                                                        \
    typedef char ArrayOfOne[1];                                         \
    typedef char ArrayOfTwo[2];                                         \
                                                                        \
    template <typename U>                                               \
    static ArrayOfOne & func(Check<int Fallback::*,&U::MEMBER_NAME> *); \
    template <typename U> static ArrayOfTwo & func(...);                \
  public:                                                               \
    typedef EMP_Detect_##MEMBER_NAME type;                              \
    enum { value = sizeof(func<Derived>(0)) == 2 };                     \
  };


// This macro will Call a member function on a given object if that member exists, but
// otherwise pass the object as an arguent to a function fallback.
//
// NEW_NAME - name of the function to be generated.
// METHOD - name of the member function that should be attempted.
// FALLBACK - function to call if no such member function exists.
// RETURN_TYPE - type that should be returned by whichever function is called.
//
// @CAO - RETURN_TYPE should be deducible.

#define EMP_CREATE_METHOD_FALLBACK(NEW_NAME, METHOD, FALLBACK, RETURN_TYPE) \
  namespace internal {                                                  \
    template <typename T, typename... ARG_TYPES>                        \
    RETURN_TYPE RelayCall_ ## NEW_NAME(typename emp::sfinae_decoy<bool, decltype(&T::METHOD)>::type, \
      T & target, ARG_TYPES... ARGS) {                                  \
    target.METHOD(ARGS...);                                             \
    }                                                                   \
    template <typename T, typename... ARG_TYPES>                        \
    RETURN_TYPE RelayCall_ ## NEW_NAME(int, T & target, ARG_TYPES... ARGS) { \
      FALLBACK(target, ARGS...);                                        \
    }                                                                   \
  }                                                                     \
  template <typename T, typename... ARG_TYPES>                          \
  RETURN_TYPE NEW_NAME(T & target, ARG_TYPES... ARGS) {                 \
    return internal::RelayCall_ ## NEW_NAME(true, target, ARGS...);     \
  }

#endif
