//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2017.
//  Released under the MIT Software license; see doc/LICENSE
//

///  These macros will build a tuple and accessors to that tuple's members inside of a
///  class definintion.
///  Status: ALPHA
///
///  "But WHY???" you ask. Let me explain: Keeping a tuple allows us to easily track the
///  members in the stuct or class, and makes possible powerful types of reflection
///  including identifying all members and performing an action on each (such as
///  serialization).  Since tuples instantiate members directly, these benefits should
///  come at no cost to performance.
///
///  To instantiate these tuple members, inside the class definition use:
///
///    `EMP_BUILD_TUPLE( int, MyInt,
///                     char, MyChar,
///                     int, MyInt2,
///                     std::string, MyString );`
///
///  This will create a std::tuple<int, char, int, std::string> and create accessors.
///  For example the accessors created for MyInt2 would look like:
///
///    `const int & MyInt2() const;
///    int & MyInt2();
///    int & MyInt2(int);`
///
///  The default tuple will be called "emp__tuple_body".  If you want to use a different
///  name for it (for example if you want more than one tuple in the class), you need to
///  call EMP_BUILD_NAMED_TUPLE, which is identical to EMP_BUILD_TUPLE, but takes the
///  name to be used for the tuple as its first argument.
///
///  Example of using an introspective tuple:
///
///    `struct JSONObject {
///    EMP_BUILD_INTROSPECTIVE_TUPLE( double, x,
///				   int, name,
/// 				   int, parent,
///                 double, y,
///                 int, depth
/// 	  	   )
///    };`
///
///  This struct can now be used as a C++ stand-in for Javscript objects for use with
///  JSWrap.
///
///  Introspective tuple structs have a static member n_fields that indicates how
///  many variables they have.
//
//  Development notes:
//  * Add static member function to help with reflection?
//  * Make play nicely with serialization techniques.
//  * It would be really awesome to make tuple structs (or at least the introspective ones)
//    be indexable and iterable. (right now the main obstacle is type inference)
//
//  Changes in last revision:
//  * Changed name of macro that builds tuple that keeps track of variable names to
//    EMP_BUILD_INTROSPECTIVE_TUPLE
//  * Introspective tuples now also keep track of variable types and maintain an
//    array of pointers to each element, which is very useful for doing operations on
//    all memebrs of a tuple.

#ifndef EMP_TUPLE_STRUCT_H
#define EMP_TUPLE_STRUCT_H

#include <tuple>
#include <typeinfo>
#include <typeindex>

#include "../base/macros.h"

///@cond SIMPLIFY
// For each entry, we want to build accessors to easily get and set each value.
#define EMP_BUILD_TUPLE_ACCESSORS(TUPLE_NAME, POS, TYPE, NAME)     \
  const TYPE & NAME() const { return std::get<POS>(TUPLE_NAME); }  \
  TYPE & NAME() { return std::get<POS>(TUPLE_NAME); }              \
  TYPE & NAME(const typename std::decay<TYPE>::type & _in) {       \
    std::get<POS>(TUPLE_NAME) = _in;                               \
    return std::get<POS>(TUPLE_NAME);                              \
  }								   \


// We have to figure out how many arguments we have.
// To get the last arg first (and know its position), reverse all arguments before sending them on.
#define EMP_BUILD_TUPLE_IMPL(TUPLE_NAME, ...) \
  EMP_ASSEMBLE_MACRO_1ARG(EMP_BUILD_TUPLE_IMPL_, TUPLE_NAME, EMP_REVERSE_ARGS(__VA_ARGS__) )

#define EMP_BUILD_TUPLE_IMPL_2(TNAME, NAME, TYPE)  \
                                             EMP_BUILD_TUPLE_ACCESSORS(TNAME, 0, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_4(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_2(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 1, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_6(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_4(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 2, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_8(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_6(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 3, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_10(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_8(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 4, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_12(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_10(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 5, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_14(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_12(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 6, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_16(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_14(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 7, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_18(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_16(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 8, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_20(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_18(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 9, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_22(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_20(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 10, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_24(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_22(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 11, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_26(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_24(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 12, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_28(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_26(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 13, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_30(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_28(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 14, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_32(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_30(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 15, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_34(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_32(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 16, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_36(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_34(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 17, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_38(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_36(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 18, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_40(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_38(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 19, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_42(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_40(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 20, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_44(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_42(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 21, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_46(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_44(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 22, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_48(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_46(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 23, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_50(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_48(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 24, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_52(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_50(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 25, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_54(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_52(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 26, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_56(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_54(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 27, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_58(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_56(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 28, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_60(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_58(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 29, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_62(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_60(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 30, TYPE, NAME)
#define EMP_BUILD_TUPLE_IMPL_64(TNAME, NAME, TYPE, ...)  \
  EMP_BUILD_TUPLE_IMPL_62(TNAME, __VA_ARGS__) EMP_BUILD_TUPLE_ACCESSORS(TNAME, 31, TYPE, NAME)


// Some debug additions!
#define EMP_BUILD_TUPLE_IMPL_0(...) { static_assert(false, "No args in BUILD_TUPLE!"); }
#define EMP_BUILD_TUPLE_IMPL_1(...) { static_assert(false, "Odd number of args in BUILD_TUPLE!"); }

/// @endcond

#define EMP_BUILD_NAMED_TUPLE(TUPLE_NAME, ...)              \
  std::tuple< EMP_GET_ODD_ARGS(__VA_ARGS__) > TUPLE_NAME;   \
  EMP_BUILD_TUPLE_IMPL(TUPLE_NAME, __VA_ARGS__)

/// See above
#define EMP_BUILD_TUPLE(...) EMP_BUILD_NAMED_TUPLE(emp__tuple_body, __VA_ARGS__)

/// @cond SIMPLIFY

//Helper macros for building introspective tuples
#define GET_TYPEID(t) typeid(t)
#define GET_POINTER(A) &A()

/// @endcond

/// Helper for EMP_BUILD_INTROSPECTIVE_TUPLE below
/// Also can be called directly if you want a tuple with a name other than
/// emp__tuple_body with stored variables.
#define EMP_BUILD_INTROSPECTIVE_NAMED_TUPLE(TUPLE_NAME, ...)            \
  const static int n_fields = EMP_COUNT_ARGS(EMP_GET_ODD_ARGS(__VA_ARGS__)); \
  std::tuple< EMP_GET_ODD_ARGS(__VA_ARGS__) > TUPLE_NAME;               \
  EMP_BUILD_TUPLE_IMPL(TUPLE_NAME, __VA_ARGS__);                        \
  std::array<std::string, EMP_COUNT_ARGS(EMP_GET_EVEN_ARGS(__VA_ARGS__))> \
  var_names{{EMP_GET_EVEN_ARGS(EMP_STRINGIFY_EACH(__VA_ARGS__))}};      \
  std::array<std::type_index, EMP_COUNT_ARGS(EMP_GET_ODD_ARGS(__VA_ARGS__))> \
  var_types{{EMP_WRAP_ARGS(GET_TYPEID, EMP_GET_ODD_ARGS(__VA_ARGS__))}}; \
  std::array<void *, EMP_COUNT_ARGS(EMP_GET_EVEN_ARGS(__VA_ARGS__))>    \
  pointers{{EMP_WRAP_ARGS(GET_POINTER, EMP_GET_EVEN_ARGS(__VA_ARGS__))}};

namespace emp {

  template <typename T>
  using n_fields_member_type = decltype(T::n_fields);
 
  template <typename T>
  constexpr bool is_introspective_tuple() { return test_type_exist<n_fields_member_type, T>(); };

}

/// This varient of the tuple building macro can be used in situations where
/// access to stringified versions of variable names is necessary.
/// It stores an array of stringified variable names called var_names in the
/// struct this macro was called from. It also stores an array containing the
/// types of all of the variables (in the same order), called var_types. Lastly,
/// It stores an array of pointers to each variable in an array called pointers.
/// It also creates a static member variable called n_fields, which is an int indicating
/// the number of variables (checking for this variable is a good way to identify
/// structs created this way for the purposes of writing generic functions).
/// This combination of meta-data allows you to do full reflection on the struct
/// created with this macro.
///
/// For instance:
/// MyStruct.var_names[0] <- returns the name of the first variable
///
/// JSWrap is capable of translating structs created with this macro to and from
/// Javascript.
///
#define EMP_BUILD_INTROSPECTIVE_TUPLE(...) EMP_BUILD_INTROSPECTIVE_NAMED_TUPLE(emp__tuple_body, __VA_ARGS__)



#endif
