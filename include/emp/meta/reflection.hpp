/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2016-2018
 *
 *  @file  reflection.hpp
 *  @brief Macros and template utilities to help determine details about unknown classes.
 */

#ifndef EMP_REFLECTION_H
#define EMP_REFLECTION_H

#include "meta.hpp"
#include "TypePack.hpp"

/// This macro will generate a function that calls a member function on a given object IF that
/// member exists, but otherwise pass the object as an arguent to a function fallback.
///
/// NEW_NAME - name of the function to be generated.
/// METHOD - name of the member function that should be attempted.
/// FALLBACK - function to call if no such member function exists.

#define EMP_CREATE_METHOD_FALLBACK(NAME, METHOD, FALLBACK)                                    \
  namespace internal {                                                                        \
    template <class T, class... ARGS>    /* T::METHOD exists! */                              \
    auto EMPCall_ ## NAME(emp::bool_decoy<decltype(&T::METHOD)>, T & target, ARGS &&... args) \
     { return target.METHOD(std::forward<ARGS>(args)...); }                                   \
    template <class T, class... ARGS>    /* T::METHOD does NOT exist! */                      \
    auto EMPCall_ ## NAME(int, T & target, ARGS &&... args)                                   \
     { return FALLBACK(target, std::forward<ARGS>(args)...); }                                \
  }                                                                                           \
  template <class T, class... ARGS> auto NAME(T & target, ARGS &&... args) {                  \
    return internal::EMPCall_ ## NAME(true, target, std::forward<ARGS>(args)...);             \
  } int ignore_semicolon_to_follow_ ## NAME = 0


/// Similar to EMP_CREATE_METHOD_FALLBACK: call method if it exists, otherwise do nothing.

#define EMP_CREATE_OPTIONAL_METHOD(NAME, METHOD)                                            \
  template <typename T, typename... ARGS>                                                   \
  void EMPCall_ ## NAME(emp::bool_decoy<decltype(&T::METHOD)>, T & target, ARGS &&... args) \
   { target.METHOD(std::forward<ARGS>(args)...); }                                          \
  template <typename T, typename... ARGS>                                                   \
  void EMPCall_ ## NAME(int, T&, ARGS...) {;}                                               \
  template <typename T, typename... ARGS> void NAME(T & target, ARGS &&... args) {          \
    EMPCall_ ## NAME(true, target, std::forward<ARGS>(args)...);                            \
  } int ignore_semicolon_to_follow_ ## NAME = 0


/// Same as above, but a return type and default value are specified.

#define EMP_CREATE_METHOD_FALLBACK_VAL(NAME, METHOD, DEFAULT)	                              \
namespace internal {                                                                        \
  template <class T, class... ARGS>    /* T::METHOD exists! */                              \
  auto EMPCall_ ## NAME(emp::bool_decoy<decltype(&T::METHOD)>, T & target, ARGS &&... args) \
   { return target.METHOD(std::forward<ARGS>(args)...); }                                   \
  auto EMPCall_ ## NAME(...)           /* T::METHOD does NOT exist! */                      \
   { return DEFAULT; }                                                                      \
}                                                                                           \
template <class T, class... ARGS> auto NAME(T & target, ARGS &&... args) {                  \
  return internal::EMPCall_ ## NAME(true, target, std::forward<ARGS>(args)...);             \
} int ignore_semicolon_to_follow_ ## NAME = 0


/// Try to perform operation EVAL1 if TEST exists, otherwise do EVAL2.
/// This is good for operations like == or << that may be defined inside
/// a class OR outside of it. (@CAO Needs more testing)

#define EMP_CREATE_EVAL_SELECT(NEW_NAME, TEST, RTYPE, EVAL1, EVAL2)  \
  template <typename... ARG_TYPES>                                   \
  RTYPE internal__RelayCall_ ## NEW_NAME(                            \
    emp::bool_decoy<decltype(TEST)>,                                 \
    ARG_TYPES... args) {                                             \
    return EVAL1(args...);                                           \
  }                                                                  \
  template <typename... ARG_TYPES>                                   \
  RTYPE internal__RelayCall_ ## NEW_NAME(int, ARG_TYPES... args) {   \
    return EVAL2(args...);                                           \
  }                                                                  \
  template <typename... ARG_TYPES>                                   \
  RTYPE NEW_NAME(ARG_TYPES... args) {                                \
    return internal__RelayCall_ ## NEW_NAME(true, args...);          \
  } int ignore_semicolon_to_follow_ ## NEW_NAME = 0


///  Build a struct which will, given a list of classes, pick the first one that has the any
///  member MEMBER defined and call that class NAME.
///
///  For example:
///    EMP_SETUP_TYPE_SELECTOR(SelectTests, is_test_type);
///    using new_type = SelectTests<S, T>
///
///  If class S has a member called test_type, this is the same as:
///    using new_type = S;
///
///  Otherwise, if S does not and T does, new_type will be T.  If neither has it, the new test_type
///  will be void.

#define EMP_SETUP_TYPE_SELECTOR(NAME, MEMBER)                                    \
template <typename T> using EMPDetect_ ## NAME = decltype(T::MEMBER);            \
template <typename... Ts>                                                        \
using NAME = typename emp::TypePack<Ts...>::template find_t<EMPDetect_ ## NAME>;


///  Given a list of classes, pick the first one that has the type MEMBER_NAME defined and
///  call that MEMBER type NAME.  If none have MEMBER_NAME, use FALLBACK_TYPE.
///
///  For example:  EMP_CHOOSE_MEMBER_TYPE(new_type, test_type, int, T);
///
///  If class T has a member type called test_type, this is the same as:
///     using new_type = T::test_type;
///
///  If T does NOT have a member type called test_type, this is the same as:
///     using new_type = int;

#define EMP_CHOOSE_MEMBER_TYPE(NAME, MEMBER, FALLBACK_T, ...)                     \
  template <typename EMP_T> using EMP_Filter_ ## NAME = typename EMP_T::MEMBER;   \
  struct EMP_Fallback_ ## NAME { using MEMBER = FALLBACK_T; };                    \
  using NAME = typename emp::TypePack<__VA_ARGS__, EMP_Fallback_ ## NAME>         \
    ::template find_t<EMP_Filter_ ## NAME>::MEMBER;


/// Return a type based on features in a class.
/// FUN = Function name
/// LEVEL = Level of overload to use, coming from bool (e.g., bool, int, ...)
/// MBR = Member function to be testing
#define EMP_IMPL_TYPE_HAS_MEMBER(FUN, LEVEL, MBR)                                   \
 template <typename EMP__T> static                                                  \
 auto FUN ## _impl(emp::sfinae_decoy<LEVEL, decltype(std::declval<EMP__T>().MBR)>)  \
     -> decltype(std::declval<EMP__T>().MBR)

#define EMP_IMPL_TYPE_HAS_TYPE(FUN, LEVEL, TYPE)                      \
 template <typename EMP__T> static                                    \
 auto FUN ## _impl(emp::sfinae_decoy<LEVEL, typename EMP__T::TYPE)    \
     -> EMP__T::TYPE

#define EMP_IMPL_TYPE_DEFAULT(FUN, LEVEL, DEFAULT)    \
 template <typename EMP__T> static DEFAULT FUN ## _impl(LEVEL)

#define EMP_ADD_TYPE_FROM_MEMBER(NEW_TYPE, BASE_TYPE, MEMBER, DEFAULT)         \
 EMP_IMPL_TYPE_HAS_MEMBER(EMP_DETECT_ ## NEW_TYPE, bool, MEMBER);              \
 EMP_IMPL_TYPE_DEFAULT(EMP_DETECT_ ## NEW_TYPE, int, DEFAULT);                 \
 using NEW_TYPE = decltype(EMP_DETECT_ ## NEW_TYPE ## _impl<BASE_TYPE>(true))

#define EMP_ADD_TYPE_FROM_TYPE(NEW_TYPE, BASE_TYPE, TYPE, DEFAULT)             \
 EMP_IMPL_TYPE_HAS_TYPE(EMP_DETECT_ ## NEW_TYPE, bool, TYPE);                  \
 EMP_IMPL_TYPE_DEFAULT(EMP_DETECT_ ## NEW_TYPE, int, DEFAULT);                 \
 using NEW_TYPE = decltype(EMP_DETECT_ ## NEW_TYPE ## _impl<BASE_TYPE>(true))

#define EMP_ADD_TYPE_FROM_MEMBER_OR_TYPE(NEW_TYPE, BASE_TYPE, MEMBER, TYPE, DEFAULT) \
 EMP_IMPL_TYPE_HAS_MEMBER(EMP_DETECT_ ## NEW_TYPE, bool, MEMBER);                    \
 EMP_IMPL_TYPE_HAS_TYPE(EMP_DETECT_ ## NEW_TYPE, int, TYPE);                         \
 EMP_IMPL_TYPE_DEFAULT(EMP_DETECT_ ## NEW_TYPE, ..., DEFAULT);                       \
 using NEW_TYPE = decltype(EMP_DETECT_ ## NEW_TYPE ## _impl<BASE_TYPE>(true))

#define EMP_ADD_TYPE_FROM_TYPE_OR_MEMBER(NEW_TYPE, BASE_TYPE, TYPE, MEMBER, DEFAULT) \
 EMP_IMPL_TYPE_HAS_TYPE(EMP_DETECT_ ## NEW_TYPE, bool, TYPE);                        \
 EMP_IMPL_TYPE_HAS_MEMBER(EMP_DETECT_ ## NEW_TYPE, int, MEMBER);                     \
 EMP_IMPL_TYPE_DEFAULT(EMP_DETECT_ ## NEW_TYPE, ..., DEFAULT);                       \
 using NEW_TYPE = decltype(EMP_DETECT_ ## NEW_TYPE ## _impl<BASE_TYPE>(true))


namespace emp {

  // Helper tools for SubsetCall.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  namespace internal {
    template <typename RETURN, typename... FUN_ARGS>
    struct SubsetCall_impl {
      template <typename... EXTRA_ARGS>
      static RETURN Call(std::function<RETURN(FUN_ARGS...)> fun, FUN_ARGS... args, EXTRA_ARGS...) {
        return fun(args...);
      }
    };
  }
  #endif // DOXYGEN_SHOULD_SKIP_THIS

  /// Identify the number of parameters in a function and pass in correct number of argument.
  template <typename RETURN, typename... FUN_ARGS, typename... CALL_ARGS>
  auto SubsetCall(std::function<RETURN(FUN_ARGS...)> fun, CALL_ARGS... args) -> RETURN {
    return internal::SubsetCall_impl<RETURN, FUN_ARGS...>::Call(fun, args...);
  }

  #ifndef DOXYGEN_SHOULD_SKIP_THIS
  // Helper tools for type_if.
  namespace internal {
    template< typename T, bool match_ok > struct EMP_eval_type { };
    template< typename T> struct EMP_eval_type<T,true> { using type = T; }; // If match, define type!
  }

  /// A template for making a type exist if and only if it passes a template test.
  /// For example: type_if<T, std::is_integral>
  /// This becomes T if the type is integral; it's undefined otherwise.
  template <typename T, template <typename...> class FILTER>
  using type_if = typename internal::EMP_eval_type< T, FILTER<T>::value >::type;
  #endif // DOXYGEN_SHOULD_SKIP_THIS
}
#endif
