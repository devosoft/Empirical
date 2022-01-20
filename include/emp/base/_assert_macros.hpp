/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file _assert_macros.hpp
 *  @brief Helper macros for building proper assert commands.
 *  @note Status: RELEASE
 *
 */

#ifndef EMP_BASE__ASSERT_MACROS_HPP_INCLUDE
#define EMP_BASE__ASSERT_MACROS_HPP_INCLUDE

/// Basic elper macros...
#define emp_assert_STRINGIFY(...) emp_assert_STRINGIFY_IMPL(__VA_ARGS__)
#define emp_assert_STRINGIFY_IMPL(...) #__VA_ARGS__
#define emp_assert_TO_PAIR(X) emp_assert_STRINGIFY(X) , X
#define emp_assert_GET_ARG_1(a, ...) a
#define emp_assert_GET_ARG_21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t, u, ...) u
#define emp_assert_MERGE(A, B) A ## B
#define emp_assert_ASSEMBLE(BASE, ARG_COUNT, ...) emp_assert_MERGE(BASE, ARG_COUNT) (__VA_ARGS__)

/// returns the number of arguments in the __VA_ARGS__; cap of 20!
#define emp_assert_COUNT_ARGS(...) emp_assert_GET_ARG_21(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/// Converts all macro arguments to pairs of arguments with both string names and values.
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

#endif // #ifndef EMP_BASE__ASSERT_MACROS_HPP_INCLUDE
