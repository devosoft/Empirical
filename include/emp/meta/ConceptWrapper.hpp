/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018-2021.
 *
 *  @file ConceptWrapper.hpp
 *  @brief A template wrapper that will either enforce functionality or provide default functions.
 *
 *  Starting in future versions of C++, a concept is a set of requirements for a class to be used
 *  in a template.  This wrapper around a class is slightly more powerful than that.  It can
 *  either REQUIRE or marks as OPTIONAL specific member functions, variables or types for a
 *  wrapped class.  REQUIRE-d members must be available in the internal class.  OPTIONAL members
 *  are giving a default that will be used for classes where that member is missing.
 *
 *  Use the EMP_BUILD_CONCEPT macro to create a new concept wrapper.  Provide it with the wrapper
 *  name, and all of the rules.  Note that since we are setting up an interface here, all of the
 *  members below are public unless otherwise stated.  The allowable rule types are:
 *
 *  REQUIRED_FUN ( FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG_TYPES... )
 *    Setup a member function called FUNCTION_NAME that is required to already be defined in the
 *    wrapped class.  If it does not exist there, trigger the ERROR_MESSAGE during compilation.
 *    The function signature is needed to automate testing if the member function exists.
 *
 *  OPTIONAL_FUN ( FUNCTION_NAME, DEFAULT_ACTION, RETURN_TYPE, ARG_TYPES... )
 *    Setup a member function called FUNCTION_NAME.  If it already exists in the wrapped class,
 *    redirect to that version when called.  If it does not already exist, perform the
 *    DEFAULT_ACTION instead (using arg1, arg2, etc as the arguments).
 *    The function signature is needed to automate testing if the member function exists.
 *
 *  REQUIRED_OVERLOAD_FUN ( FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG1_TYPES, OTHER_ARGS... )
 *    Setup a set of overloaded member functions called FUNCTION_NAME that varies the first
 *    parameter (and may have additional paramters with fixed types.  ARG1_TYPES must be an
 *    emp::TypePack that includes the full set of types to be used for the first parameter.
 *    Zero or more additional parameters may be included in OTHER_ARGS.  The wrapped class must
 *    already define the full set of overloaded functions by the correct name and with the correct
 *    RETURN_TYPE or else the ERROR_MESSAGE will be triggered.
 *    Example:
 *      REQUIRED_OVERLOAD_FUN ( CountValues, "Missing templated CountValues!",
 *                              int, emp::TypePack<int, char, double> );
 *    This will allow CountValues to be correctly redirected if called with any of the three
 *    specified types as its one parameter.
 *
 *  REQUIRED_TYPE ( TYPE_NAME, ERROR_MESSAGE )
 *    Setup a member type called TYPE_NAME that is required to be defined in the wrapped class.
 *    If it does not exist there, trigger the ERROR_MESSAGE during compilation.
 *
 *  OPTIONAL_TYPE ( TYPE_NAME, DEFAULT_TYPE )
 *    Setup a member type called TYPE_NAME.  If it already exists in the wrapped class, use that
 *    version.  Otherwise set it to DEFAULT_TYPE provided.
 *
 *  PRIVATE ( CODE )
 *    All code provided will appear in the private portion of the wrapper.  Any identifiers
 *    defined in this code will shadow public defined in the class being wrapped.
 *
 *  PROTECTED ( CODE )
 *    All code provided will appear in the protected portion of the wrapper.  Any identifiers
 *    defined in this code will shadow public defined in the class being wrapped.
 *
 *  PUBLIC ( CODE )
 *    All code provided will appear in the public portion of the wrapper.  Any identifiers
 *    defined in this code will shadow public defined in the class being wrapped.
 *
 *
 *  ----------------
 *  DEVELOPER NOTES:
 *    Internally, the base class declares all required and optional functions a pure virtual.
 *
 *    The wrapper is a templated class that takes in the wrapped class and inherits from BOTH
 *    the base class and the wrapped class.
 *
 *    When a function is called in the base class, it automatically redirects to the derived
 *    class (via normal virtual functionality), which will then either redirect the call to the
 *    wrapped class or (if optional and not implemented in the wrapped class) executes the default
 *    code.
 *
 *
 *  @note: Requires C++-17 to function properly!
 *
 *  @todo: Add the ability to rename functions from the base class.
 *  @todo: Add the ability to list several functions, requiring only one to exist.
 *         (This can be done by surrounding all names in parens to build a pack)
 *
 *
 *  Currently not implementing vars (to encourage accessor-based interface), but it would look like:
 *
 *  REQUIRED_VAR ( VAR_NAME, ERROR_MESSAGE, TYPE )
 *    Setup a member variable called VAR_NAME that is required to already be declared in the
 *    wrapped class.  If it does not exist there, trigger the ERROR_MESSAGE during compilation.
 *
 *  OPTIONAL_VAR ( VAR_NAME, DEFAULT_VALUE, TYPE )
 *    Setup a member variable called VAR_NAME.  If it already exists in the wrapped class, use
 *    that version.  If it does not already exist, create it with the provided TYPE and set it to
 *    the DEFAULT_VALUE prodided.
 *
 */

#ifndef EMP_META_CONCEPTWRAPPER_HPP_INCLUDE
#define EMP_META_CONCEPTWRAPPER_HPP_INCLUDE

#include <string>
#include <utility>

#include "macros.hpp"
#include "meta.hpp"
#include "TypePack.hpp"



#define EMP_BUILD_CONCEPT( WRAPPER_NAME, BASE_NAME, ... )      \
  /* Do error-checkig on the inputs! */                        \
  EMP_WRAP_EACH(EMP_BUILD_CONCEPT__ERROR_CHECK, __VA_ARGS__)   \
  /* Build the interface class. */                             \
  class BASE_NAME {                                            \
  public:                                                      \
    EMP_WRAP_EACH(EMP_BUILD_CONCEPT__BASE, __VA_ARGS__)        \
  };                                                           \
  /* Build the wrapper class. */                               \
  template <typename WRAPPED_T>                                \
  class WRAPPER_NAME : public WRAPPED_T, public BASE_NAME {    \
    using this_t = WRAPPER_NAME<WRAPPED_T>;                    \
    EMP_WRAP_EACH(EMP_BUILD_CONCEPT__PROCESS, __VA_ARGS__)     \
  }


// ***************************************************************************
// We want to do error checking to minimize confusing errors that come out of the compiler.

// Any legal entry should be made empty.  Complain if anything is left!
#define EMP_BUILD_CONCEPT__ERROR_CHECK( CMD ) EMP_BUILD_CONCEPT__ERROR_CHECK_impl(EMP_BUILD_CONCEPT__EC_ ## CMD, CMD)
#define EMP_BUILD_CONCEPT__ERROR_CHECK_impl( RESULT, CMD ) EMP_BUILD_CONCEPT__CHECK_EMPTY( RESULT, CMD )

#define EMP_BUILD_CONCEPT__EC_REQUIRED_FUN(...)           /* REQUIRED_FUN okay */
#define EMP_BUILD_CONCEPT__EC_OPTIONAL_FUN(...)           /* OPTIONAL_FUN okay */
#define EMP_BUILD_CONCEPT__EC_REQUIRED_OVERLOAD_FUN(...)  /* REQUIRED_OVERLOAD_FUN okay */
#define EMP_BUILD_CONCEPT__EC_REQUIRED_TYPE(...)          /* REQUIRED_TYPE okay */
#define EMP_BUILD_CONCEPT__EC_OPTIONAL_TYPE(...)          /* OPTIONAL_TYPE okay */
#define EMP_BUILD_CONCEPT__EC_PRIVATE(...)                /* PRIVATE okay */
#define EMP_BUILD_CONCEPT__EC_PROTECTED(...)              /* PROTECTED okay */
#define EMP_BUILD_CONCEPT__EC_PUBLIC(...)                 /* PUBLIC okay */

#define EMP_BUILD_CONCEPT__CHECK_EMPTY(A, CMD)  EMP_GET_ARG_2( EMP_BUILD_CONCEPT__SPACER ## A, \
          static_assert(false, "\n\n  \033[1;31mInvalid EMP_BUILD_CONCEPT.\033[0m May be invalid command or missing comma in:\n    \033[1;32m" #CMD "\033[0m;\n\n"); )
#define EMP_BUILD_CONCEPT__SPACER ~, /* EMPTY! */
#define EMP_BUILD_CONCEPT__ERROR


// ***************************************************************************
// Build the base class for concepts.

#define EMP_BUILD_CONCEPT__BASE( CMD ) EMP_BUILD_CONCEPT__BASE_ ## CMD

// Required and optional functions should have a virtual version declared in the base class.
#define EMP_BUILD_CONCEPT__BASE_REQUIRED_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;
#define EMP_BUILD_CONCEPT__BASE_OPTIONAL_FUN(NAME, X, RETURN_T, ...) virtual RETURN_T NAME( __VA_ARGS__ ) = 0;

// Since you cannot have virtual tempalated functions, we need to do a bit of work in the bast class.
// ARGS are: FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG1_TYPES, OTHER_ARGS...
#define EMP_BUILD_CONCEPT__BASE_REQUIRED_OVERLOAD_FUN(NAME, X, RETURN_TYPE, ...)         \
  static_assert(emp::is_TypePack<TYPE_OPTIONS>() == true,                                \
      "Use TypeTrait for call types to require overloaded function in ConceptWrapper."); \
  using NAME ## __overloads_t = EMP_GET_ARG(1, __VA_ARGS__);                             \
  using NAME ## __padded_t = NAME ## __overloads_t;                             \
  /* Build the actual functions that we need to redirect a call to a derived type. */    \
  template <typename T>                                                                  \
  auto NAME( T && arg ) {                                                                \
    const size_t this_type_id = EMP_derived_type_id__ ## NAME;                           \
    auto & target_fun = EMP_derived_connector__ ## NAME<T>::Get(this_type_id);           \
    return target_fun( this, std::forward<T>(arg) );                                     \
  }

/////    constexpr size_t arg_type_id = EMP_template_types__ ## NAME :: GetID<T>();


// Other types don't need any modification of the base class (empty macros prevent compiler errors)
#define EMP_BUILD_CONCEPT__BASE_REQUIRED_TYPE(...)
#define EMP_BUILD_CONCEPT__BASE_OPTIONAL_TYPE(...)
#define EMP_BUILD_CONCEPT__BASE_PRIVATE(...)
#define EMP_BUILD_CONCEPT__BASE_PROTECTED(...)
#define EMP_BUILD_CONCEPT__BASE_PUBLIC(...)


// ***************************************************************************
// Setup the wrapper.

#define EMP_BUILD_CONCEPT__PROCESS( CMD ) EMP_BUILD_CONCEPT__PROCESS_ ## CMD

#define EMP_BUILD_CONCEPT__PROCESS_REQUIRED_FUN(FUN_NAME, ERROR, ...)                             \
  EMP_BUILD_CONCEPT__REQUIRED_FUN_impl(FUN_NAME, ERROR,                                           \
                                       EMP_DEC(EMP_COUNT_ARGS(__VA_ARGS__)), /* How many args? */ \
                                       EMP_GET_ARG(1, __VA_ARGS__),          /* Return type */    \
                                       EMP_POP_ARG(__VA_ARGS__) )            /* Arg types */

#define EMP_BUILD_CONCEPT__REQUIRED_FUN_impl(FUN_NAME, ERROR, NUM_ARGS, RETURN_T, ...)            \
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

// ARGS are: FUNCTION_NAME, ERROR_MESSAGE, RETURN_TYPE, ARG1_TYPES, OTHER_ARGS...
#define EMP_BUILD_CONCEPT__PROCESS_REQUIRED_OVERLOAD_FUN(FUN_NAME, ERROR_MESSAGE, RETURN_T, ...)  \
  protected:                                                                                      \
    /* Determine return type if we try to call this function in the base class.                   \
       It should be undefined if the member functon does not exist!                           */  \
    template <typename WRAPPED_T, typename ARG_T>                                                 \
    using return_t_ ## FUN_NAME =                                                                 \
      decltype( std::declval<WRAPPED_T>().FUN_NAME( std::declval<ARG_T>() ) );                    \
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

#define EMP_BUILD_CONCEPT__PROCESS_REQUIRED_TYPE(TYPE_NAME, ERROR)                                \
  protected:                                                                                      \
    /* Test if a type defines another type.                                                   */  \
    template <typename T>                                                                         \
    using has_t_ ## TYPE_NAME = typename T::TYPE_NAME;                                            \
  public:                                                                                         \
    /* Test whether type exists in the base class, based on SFINAE in using return type.      */  \
    static constexpr bool HasType_ ## TYPE_NAME() {                                               \
      constexpr bool result = emp::test_type<has_t_ ## TYPE_NAME, WRAPPED_T>();                   \
      static_assert( result, "\n\n  ** " ERROR " (Class: " "TBD" ") **\n" );                      \
      return result;                                                                              \
    }                                                                                             \
    using TYPE_NAME = typename WRAPPED_T::TYPE_NAME;

#define EMP_BUILD_CONCEPT__PROCESS_OPTIONAL_TYPE(TYPE_NAME, DEFAULT_T)                            \
  protected:                                                                                      \
    /* Test if a type defines another type.                                                   */  \
    template <typename T>                                                                         \
    using has_t_ ## TYPE_NAME = typename T::TYPE_NAME;                                            \
  public:                                                                                         \
    /* Test whether type exists in the base class, based on SFINAE in using return type.      */  \
    static constexpr bool HasType_ ## TYPE_NAME() {                                               \
      return emp::test_type<has_t_ ## TYPE_NAME, WRAPPED_T>();                                    \
    }                                                                                             \
    using TYPE_NAME = typename emp::TypePack<WRAPPED_T>::template wrap<has_t_ ## TYPE_NAME>::template push_back<DEFAULT_T>::first_t;

#define EMP_BUILD_CONCEPT__PROCESS_PRIVATE(...) private: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PUBLIC(...) public: __VA_ARGS__
#define EMP_BUILD_CONCEPT__PROCESS_PROTECTED(...) protected: __VA_ARGS__



#endif // #ifndef EMP_META_CONCEPTWRAPPER_HPP_INCLUDE
