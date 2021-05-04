/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file always_assert.hpp
 *  @brief A more dynamic replacement for standard library asserts.
 *  @note Status: RELEASE
 *
 *  A replacement for the system-level assert.h, called "emp_always_assert"
 *  Added functionality:
 *   - If compiled with Emscripten, will provide pop-up alerts in a web browser.
 *   - emp_assert can take additional arguments.  If the assert is triggered,
 *     those extra arguments will be evaluated and printed.
 *   - if EMP_TDEBUG is defined, emp_assert() goes into test mode and records
 *     failures, but does not abort.  (useful for unit tests of asserts)
 *
 *  Example:
 *
 *     int a = 6;
 *     emp_always_assert(a==5, a);
 *
 *  Unlinke "emp_assert", "emp_always_assert" will trigger an assertion error
 *  whether compiled in debug mode or not.
 *
 */

#ifndef EMP_ALWAYS_ASSERT_HPP
#define EMP_ALWAYS_ASSERT_HPP

#include <cstdlib>

#include "_assert_trigger.hpp"

/// Helper macros...
#define emp_assert_STRINGIFY(...) emp_assert_STRINGIFY_IMPL(__VA_ARGS__)
#define emp_assert_STRINGIFY_IMPL(...) #__VA_ARGS__
#define emp_assert_TO_PAIR(X) emp_assert_STRINGIFY(X) , X
#define emp_assert_GET_ARG_1(a, ...) a
#define emp_assert_GET_ARG_21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t, u, ...) u
#define emp_assert_MERGE(A, B) A ## B
#define emp_assert_ASSEMBLE(BASE, ARG_COUNT, ...) emp_assert_MERGE(BASE, ARG_COUNT) (__VA_ARGS__)


/// returns the number of arguments in the __VA_ARGS__; cap of 20!
#define emp_assert_COUNT_ARGS(...) emp_assert_GET_ARG_21(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define emp_assert_TO_PAIRS(...) emp_assert_ASSEMBLE(emp_assert_TO_PAIRS, emp_assert_COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

#define emp_assert_TO_PAIRS1(X) emp_assert_TO_PAIR(X)
#define emp_assert_TO_PAIRS2(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS1(__VA_ARGS__)
#define emp_assert_TO_PAIRS3(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS2(__VA_ARGS__)
#define emp_assert_TO_PAIRS4(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS3(__VA_ARGS__)
#define emp_assert_TO_PAIRS5(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS4(__VA_ARGS__)
#define emp_assert_TO_PAIRS6(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS5(__VA_ARGS__)
#define emp_assert_TO_PAIRS7(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS6(__VA_ARGS__)
#define emp_assert_TO_PAIRS8(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS7(__VA_ARGS__)
#define emp_assert_TO_PAIRS9(X, ...)  emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS8(__VA_ARGS__)
#define emp_assert_TO_PAIRS10(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS9(__VA_ARGS__)

#define emp_assert_TO_PAIRS11(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS10(__VA_ARGS__)
#define emp_assert_TO_PAIRS12(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS11(__VA_ARGS__)
#define emp_assert_TO_PAIRS13(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS12(__VA_ARGS__)
#define emp_assert_TO_PAIRS14(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS13(__VA_ARGS__)
#define emp_assert_TO_PAIRS15(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS14(__VA_ARGS__)
#define emp_assert_TO_PAIRS16(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS15(__VA_ARGS__)
#define emp_assert_TO_PAIRS17(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS16(__VA_ARGS__)
#define emp_assert_TO_PAIRS18(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS17(__VA_ARGS__)
#define emp_assert_TO_PAIRS19(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS18(__VA_ARGS__)
#define emp_assert_TO_PAIRS20(X, ...) emp_assert_TO_PAIR(X) , emp_assert_TO_PAIRS19(__VA_ARGS__)

#if defined( __EMSCRIPTEN__ )

  #define emp_always_assert_impl(...)                                     \
    do {                                                                  \
      !(emp_assert_GET_ARG_1(__VA_ARGS__, ~))                             \
      && emp::assert_trigger(                                             \
        __FILE__, __LINE__,                                               \
        emp_assert_STRINGIFY( emp_assert_GET_ARG_1(__VA_ARGS__, ~),  ),   \
        emp_assert_TO_PAIRS(__VA_ARGS__)                                  \
      );                                                                  \
    } while(0)

#elif defined( _MSC_VER )

  #define emp_always_assert_msvc_impl(TEST)                               \
    do {                                                                  \
      !(TEST)                                                             \
      && emp::assert_trigger(__FILE__, __LINE__, #TEST, 0)                \
      && (std::abort(), false);                                           \
    } while(0)

  #define emp_always_assert_impl(TEST) emp_always_assert_msvc_impl(TEST)

#else

  #define emp_always_assert_impl(...)                                     \
    do {                                                                  \
      !(emp_assert_GET_ARG_1(__VA_ARGS__, ~))                             \
      && emp::assert_trigger(                                             \
        __FILE__, __LINE__,                                               \
        emp_assert_STRINGIFY( emp_assert_GET_ARG_1(__VA_ARGS__, ~) ),     \
        emp_assert_TO_PAIRS(__VA_ARGS__)                                  \
      )                                                                   \
      && (std::abort(), false);                                           \
    } while(0)

#endif

/// Require a specified condition to be true. If it is false, immediately
/// halt execution. Print also extra information on any variables or
/// expressions provided as variadic args. Will be evaluated when compiled in
/// both debug and release mode.
#define emp_always_assert(...) emp_always_assert_impl(__VA_ARGS__)

#endif // #ifdef EMP_ALWAYS_ASSERT_HPP
