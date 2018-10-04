/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  ConceptWrapper.h
 *  @brief A template wrapper that will either enforce functionality or provide default functions.
 *
 *  Starting in future versions of C++, a concept is a set of requirements for a class to be used
 *  in a template.  This wrapper around a class can either REQUIRE functions to be present in the
 *  internal class, or provide DEFAULT functionality when functions are missing.
 * 
 *  Use the EMP_BUILD_CONCEPT macro to create a new concept wrapper.  Provide it with the wrapper
 *  name, and all of the rules.  The allowable rule types are:
 *  
 *  REQUIRED_FUN ( FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG_TYPES... )
 *    Setup a function that is required to be present in the wrapped class.  If it does not
 *    exist there, throw the provided error.
 * 
 *  OPTIONAL_FUN ( FUNCTION_NAME, DEFAULT_ACTION, RETURN_TYPE, ARG_TYPES... )
 *    Setup a function.  If it exists, call the version in the wrapped class.  If it does not
 *    exist, return the default instead.  The function signature is needed as part of the 
 *    automated testing as to whether the function exists.
 * 
 *  PRIVATE ( CODE )
 *    All code provided will appear in the private portion of the wrapper.
 * 
 *  PROTECTED ( CODE )
 *    All code provided will appear in the protected portion of the wrapper.
 *  
 *  PUBLIC ( CODE )
 *    All code provided will appear in the public portion of the wrapper.
 * 
 * 
 *  @note: Requires C++-17 to function properly!
 * 
 *  @todo: Add the ability to rename functions from the base class.
 *  @todo: Add the ability to list several functions, requiring only one to exist.
 *         (This can be done by surrounding all names in parens to build a pack)
 */

#ifndef EMP_CONCEPT_WRAPPER_H
#define EMP_CONCEPT_WRAPPER_H

#include <string>
#include <utility>

#include "../base/macros.h"
#include "meta.h"

#define EMP_BC__WRAP_EACH(W, ...) EMP_BC__CALL_BY_PACKS(EMP_BC__WRAP_EACH_, W, __VA_ARGS__)
#define EMP_BC__WRAP_EACH_1(W, A, ...) W(A)
#define EMP_BC__WRAP_EACH_2(W, A,B,...) EMP_BC__WRAP_EACH_1(W, A, ~) EMP_BC__WRAP_EACH_1(W, B, ~)
#define EMP_BC__WRAP_EACH_4(W, A,B,...) EMP_BC__WRAP_EACH_2(W, A, B, ~) EMP_BC__WRAP_EACH_2(W, __VA_ARGS__)
#define EMP_BC__WRAP_EACH_8(W, ...)                                         \
  EMP_BC__WRAP_EACH_4(W, __VA_ARGS__)                                       \
  EMP_BC__EVAL1( EMP_BC__WRAP_EACH_4 EMP_BC__EMPTY() (W, EMP_POP_ARGS_4(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_16(W, ...) \
  EMP_BC__WRAP_EACH_8(W, __VA_ARGS__) \
  EMP_BC__EVAL2( EMP_BC__WRAP_EACH_8 EMP_BC__EMPTY() (W, EMP_POP_ARGS_8(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_32(W, ...) \
  EMP_BC__WRAP_EACH_16(W, __VA_ARGS__) \
  EMP_BC__EVAL3( EMP_BC__WRAP_EACH_16 EMP_BC__EMPTY() (W, EMP_POP_ARGS_16(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_64(W, ...) \
  EMP_BC__WRAP_EACH_32(W, __VA_ARGS__) \
  EMP_BC__EVAL4( EMP_BC__WRAP_EACH_32 EMP_BC__EMPTY() (W, EMP_POP_ARGS_32(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_128(W, ...) \
  EMP_BC__WRAP_EACH_64(W, __VA_ARGS__) \
  EMP_BC__EVAL5( EMP_BC__WRAP_EACH_64 EMP_BC__EMPTY() (W, EMP_POP_ARGS_64(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_256(W, ...) \
  EMP_BC__WRAP_EACH_128(W, __VA_ARGS__) \
  EMP_BC__EVAL6( EMP_BC__WRAP_EACH_128 EMP_BC__EMPTY() (W, EMP_POP_ARGS_128(__VA_ARGS__)) )
#define EMP_BC__WRAP_EACH_512(W, ...) \
  EMP_BC__WRAP_EACH_256(W, __VA_ARGS__) \
  EMP_BC__EVAL7( EMP_BC__WRAP_EACH_256 EMP_BC__EMPTY() (W, EMP_POP_ARGS_256(__VA_ARGS__)) )


/// EMP_BC__CALL_BY_PACKS is used to build other macros.  It will call a series of versions of C
/// based on binary representations so that all args are called, passing F in as the first
/// parameter.  For example, if C = ABC_ and 13 arguments are passed in, it will call ABC_8,
/// ABC_4 and ABC_1 on appropriate subsets (prepending F as the first argument of each call)
///
/// C is the CALL needed to be made on each element of the parameter pack
/// F is a FIXED parameter (potentially a pack) sent to all calls.
#define EMP_BC__CALL_BY_PACKS(C, F, ...)                                     \
  EMP_BC__CALL_BY_PACKS_impl(C, F, EMP_DEC_TO_PACK(EMP_COUNT_ARGS(__VA_ARGS__)), __VA_ARGS__, ~)

/// @cond MACROS

// Internal helpers...
// P is the pack of call counts the still need to be done
// A is the number of arguments in P.
// N is the NEXT call count needed to be done.
#define EMP_BC__CALL_BY_PACKS_impl(C, F, P, ...) \
  EMP_BC__CALL_BY_PACKS_implB(C, F, EMP_BC__PACK_SIZE(P), EMP_BC__PACK_PUSH_REAR(~, P), __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_implB(C, F, A, P, ...) EMP_BC__CALL_BY_PACKS_implC(C, F, A, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_implC(C, F, A, P, ...) \
  EMP_BC__CALL_BY_PACKS_implD(C, EMP_BC__PACK_TOP(P), F, A, EMP_BC__PACK_POP(P), __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_implD(C, N, F, A, P, ...) EMP_BC__CALL_BY_PACKS_impl##A(C,N,F,P,__VA_ARGS__)

#define EMP_BC__CALL_BY_PACKS_impl1(C, N, F, P, ...)        \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)

#define EMP_BC__CALL_BY_PACKS_impl2(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl2B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl2B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl1(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl3(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl3B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl3B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl2(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl4(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl4B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl4B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl3(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl5(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl5B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl5B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl4(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl6(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl6B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl6B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl5(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl7(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl7B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl7B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl6(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl8(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl8B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl8B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl7(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl9(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl9B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl9B(C, N, F, P, ...)                       \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl8(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_impl10(C, N, F, P, ...) EMP_BC__CALL_BY_PACKS_impl10B(C, N, F, P, __VA_ARGS__)
#define EMP_BC__CALL_BY_PACKS_impl10B(C, N, F, P, ...)                      \
  EMP_BC__CALL_BY_PACKS_do_call(C, N, F, __VA_ARGS__)                       \
  EMP_BC__CALL_BY_PACKS_impl9(C, EMP_BC__PACK_TOP(P), F, EMP_BC__PACK_POP(P), EMP_POP_ARGS_ ## N(__VA_ARGS__) )

#define EMP_BC__CALL_BY_PACKS_do_call(C, V, F, ...) C ## V(F, __VA_ARGS__)

#define EMP_BC__ECHO_ARGS(...) __VA_ARGS__
#define EMP_BC__PACK_ARGS(...) (__VA_ARGS__)
#define EMP_BC__UNPACK_ARGS(A) EMP_BC__ECHO_ARGS A
#define EMP_BC__PACK_POP(PACK) (EMP_POP_ARG PACK)
#define EMP_BC__PACK_TOP(PACK) EMP_GET_ARG_1 PACK
#define EMP_BC__PACK_PUSH(NEW, PACK) (NEW,EMP_BC__UNPACK_ARGS(PACK))
#define EMP_BC__PACK_PUSH_REAR(NEW, PACK) (EMP_BC__UNPACK_ARGS(PACK),NEW)
#define EMP_BC__PACK_SIZE(PACK) EMP_COUNT_ARGS PACK




#define EMP_BUILD_CONCEPT( CLASS_NAME, ... )                         \
  /* Build the interface class. */                                   \
  class CLASS_NAME ## _Base {                                        \
  public:                                                            \
    EMP_BC__WRAP_EACH(EMP_BUILD_CONCEPT__BASE, __VA_ARGS__)              \
  };                                                                 \
  /* Build the wrapper class. */                                     \
  template <typename WRAPPED_T>                                      \
  class CLASS_NAME : public WRAPPED_T, public CLASS_NAME ## _Base {  \
    using this_t = CLASS_NAME<WRAPPED_T>;                            \
    EMP_BC__WRAP_EACH(EMP_BUILD_CONCEPT__PROCESS, __VA_ARGS__)           \
  }

#define EMP_BUILD_CONCEPT__BASE( CMD ) EMP_BUILD_CONCEPT_BASE__ ## CMD

#define EMP_BUILD_CONCEPT_BASE__REQUIRED_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;
#define EMP_BUILD_CONCEPT_BASE__OPTIONAL_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;
#define EMP_BUILD_CONCEPT_BASE__PRIVATE(...)
#define EMP_BUILD_CONCEPT_BASE__PROTECTED(...)
#define EMP_BUILD_CONCEPT_BASE__PUBLIC(...)


#define EMP_BUILD_CONCEPT__PROCESS( CMD ) EMP_BUILD_CONCEPT__PROCESS_ ## CMD

#define EMP_BUILD_CONCEPT__PROCESS_REQUIRED_FUN(FUN_NAME, ERROR, ...)                             \
  EMP_BUILD_CONCEPT__REQUIRED_impl(FUN_NAME, ERROR,                                               \
                                   EMP_DEC(EMP_COUNT_ARGS(__VA_ARGS__)), /* How many args? */     \
                                   EMP_GET_ARG(1, __VA_ARGS__),          /* Return type */        \
                                   EMP_POP_ARG(__VA_ARGS__) )            /* Arg types */

#define EMP_BUILD_CONCEPT__REQUIRED_impl(FUN_NAME, ERROR, NUM_ARGS, RETURN_T, ...)                \
  protected:                                                                                      \
    /* Determine return type if we try to call this function in the base class.                   \
       It should be undefined if the member functon does not exist!                           */  \
    template <typename T>                                                                         \
    using return_t_ ## FUN_NAME =                                                                 \
      EMP_IF( NUM_ARGS,                                                                           \
        decltype( std::declval<T>().FUN_NAME( EMP_TYPES_TO_VALS(__VA_ARGS__) ) );,                \
        decltype( std::declval<T>().FUN_NAME() );                                                 \
      )                                                                                           \
  public:                                                                                         \
    /* Test whether function exists, based on SFINAE in using return type.                    */  \
    static constexpr bool HasFun_ ## FUN_NAME() {                                                 \
      return emp::test_type<return_t_ ## FUN_NAME, WRAPPED_T>();                                  \
    }                                                                                             \
    static constexpr size_t ArgCount_ ## FUN_NAME = NUM_ARGS;                                     \
    /* Call appropriate version of the function.  First determine if there is a non-void          \
       return type (i.e., do we return th result?) then check if the function exists in the       \
       wrapped class or should we call/return the default (otherwise).                        */  \
    EMP_IF( NUM_ARGS,                                                                             \
            RETURN_T FUN_NAME( EMP_DECLARE_VARS(__VA_ARGS__) ),                                   \
            RETURN_T FUN_NAME( )                                                                  \
    ) {                                                                                           \
      static_assert( HasFun_ ## FUN_NAME(), "\n\n  ** " ERROR " (Class: " "TBD" ") **\n" );       \
      if constexpr (HasFun_ ## FUN_NAME()) {                                                      \
        EMP_IF( EMP_TEST_IF_VOID(RETURN_T),                                                       \
          /* void return -> call function, but don't return result. */                            \
          { WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); },                                 \
          /* non-void return -> make sure to return result. */                                    \
          { return WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }                           \
        )                                                                                         \
      }                                                                                           \
    }


/// Macro to dynamically call an OPTIONAL function; it will call the version in the wrapped type
/// if it exists, or else return the default provided (which can be either a function to call or a
/// value.  The first two arguments are the function name and its default return.  The remaining
/// arguments in the ... must be the return type (required) and all argument types (if any exist)

#define EMP_BUILD_CONCEPT__PROCESS_OPTIONAL_FUN(NAME, DEFAULT, ...)                               \
  EMP_BUILD_CONCEPT__OPTIONAL_impl(NAME, DEFAULT,                                                 \
                                   EMP_DEC(EMP_COUNT_ARGS(__VA_ARGS__)),  /* How many args? */    \
                                   EMP_GET_ARG(1, __VA_ARGS__),           /* Return type */       \
                                   EMP_POP_ARG(__VA_ARGS__) )             /* Arg types */

#define EMP_BUILD_CONCEPT__OPTIONAL_impl(FUN_NAME, DEFAULT, NUM_ARGS, RETURN_T, ...)              \
  protected:                                                                                      \
    /* Determine return type if we try to call this function in the base class.                   \
       It should be undefined if the member functon does not exist!                           */  \
    template <typename T>                                                                         \
    using return_t_ ## FUN_NAME =                                                                 \
      EMP_IF( NUM_ARGS,                                                                           \
        decltype( std::declval<T>().FUN_NAME(EMP_TYPES_TO_VALS(__VA_ARGS__)) );,                  \
        decltype( std::declval<T>().FUN_NAME() );                                                 \
      )                                                                                           \
  public:                                                                                         \
    /* Test whether function exists, based on SFINAE in using return type.                    */  \
    static constexpr bool HasFun_ ## FUN_NAME() {                                                 \
      return emp::test_type<return_t_ ## FUN_NAME, WRAPPED_T>();                                  \
    }                                                                                             \
    static constexpr size_t ArgCount_ ## FUN_NAME = NUM_ARGS;                                     \
    /* Call appropriate version of the function.  First determine if there is a non-void          \
       return type (i.e., do we return th result?) then check if the function exists in the       \
       wrapped class or should we call/return the default (otherwise).                        */  \
    EMP_IF( NUM_ARGS,                                                                             \
            RETURN_T FUN_NAME( EMP_DECLARE_VARS(__VA_ARGS__) ),                                   \
            RETURN_T FUN_NAME( )                                                                  \
    ) {                                                                                           \
      constexpr bool has_fun = HasFun_ ## FUN_NAME();                                             \
      EMP_IF( EMP_TEST_IF_VOID(RETURN_T),                                                         \
        { /* void return -> call function, but don't return result. */                            \
          if constexpr (has_fun) { WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }           \
          else { DEFAULT; }                                                                       \
        },                                                                                        \
        { /* non-void return -> make sure to return result. */                                    \
          if constexpr (has_fun) { return WRAPPED_T::FUN_NAME( EMP_NUMS_TO_VARS(NUM_ARGS) ); }    \
          else { return DEFAULT; }                                                                \
        }                                                                                         \
      )                                                                                           \
    }

#define EMP_BUILD_CONCEPT__PROCESS_PRIVATE(...) private: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PUBLIC(...) public: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PROTECTED(...) protected: __VA_ARGS__


#endif

