#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is a replacement for the system-level assert.h, called "emp_assert"
//  It behaves nearly identically, but with some additional functionality:
//  - If compiled with Emscripten, will provide pop-up alerts in a web browser.
//  - emp_assert can take additional arguments.  If the assert is triggered, those arguments
//    will be evaluated and printed.
//  - if NDEBUG -or- EMP_NDEBUG is defined, the expression in emp_assert() is not evaluated.
//  - if EMP_TDEBUG is defined, emp_assert() goes into test mode and records failures, but
//    does not abort.  (useful for unit tests of asserts)
//

#include <iostream>
#include <string>
#include <sstream>

#include "macros.h"

// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

// NDEBUG and TDEBUG should trigger their EMP equivilents.
#ifdef NDEBUG
#define EMP_NDEBUG
#endif

#ifdef TDEBUG
#define EMP_TDEBUG
#endif


#ifdef EMP_NDEBUG
namespace emp {
  const bool assert_on = false;
  int assert_count = 0;
}

// Ideally, this assert should use the expression (to prevent compiler error), but should not
// generate any assembly code.  For now, just make it blank (other options commented out)
#define emp_assert(...)
// #define emp_assert(EXPR) ((void) sizeof(EXPR) )
// #define emp_assert(EXPR, ...) { constexpr bool __emp_assert_tmp = false && (EXPR); (void) __emp_assert_tmp; }

#elif defined(EMP_TDEBUG)           // EMP_NDEBUG not set, but EMP_TDEBUG is!

namespace emp {
  const bool assert_on = true;
  struct AssertFailInfo {
    std::string filename;
    int line_num;
    std::string error;
  };
  AssertFailInfo assert_fail_info;
  bool assert_last_fail = false;
}

// Generate a pop-up alert in a web browser if an assert is tripped.
#define emp_assert_tdebug_impl(EXPR) emp_assert_tdebug_impl2(EXPR)

#define emp_assert_tdebug_impl2(EXPR)                                    \
  do {                                                                  \
    if ( !(EXPR) ) {                                                    \
      emp::assert_last_fail = true;                                     \
      emp::assert_fail_info.filename = __FILE__;                        \
      emp::assert_fail_info.line_num = __LINE__;                        \
      emp::assert_fail_info.error = #EXPR;                              \
    }                                                                   \
    else {                                                              \
      emp::assert_last_fail = false;                                    \
    }                                                                   \
  } while (0)

#define emp_assert(...) emp_assert_tdebug_impl( EMP_GET_ARG_1(__VA_ARGS__, ~) )


#elif EMSCRIPTEN  // Neither EMP_NDEBUG nor EMP_TDEBUG set, but compiling with Emscripten

namespace emp {
  const bool assert_on = true;
  static int TripAssert() {
    static int trip_count = 0;
    return ++trip_count;
  }
}

// Generate a pop-up alert in a web browser if an assert it tripped.
#define emp_assert_impl_1(EXPR)                                         \
  if ( !(EXPR) ) {                                                      \
    std::string msg = std::string("Assert Error (In ")                  \
      + std::string(__FILE__)                                           \
      + std::string(" line ") + std::to_string(__LINE__)                \
      + std::string("): ") + std::string(#EXPR) + "\n"                  \
      + emp_assert_var_info.str();                                      \
    if (emp::TripAssert() <= 3)						\
      EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str()); \
    abort();                                                            \
  }                                                                     \
  
#define emp_assert_var(VAR) emp_assert_var_info << #VAR << ": [" << VAR << "]\n";

#define emp_assert_impl_2(EXPR, VAR) emp_assert_var(VAR); emp_assert_impl_1(EXPR)
#define emp_assert_impl_3(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_2(EXPR,__VA_ARGS__)
#define emp_assert_impl_4(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_3(EXPR,__VA_ARGS__)
#define emp_assert_impl_5(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_4(EXPR,__VA_ARGS__)
#define emp_assert_impl_6(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_5(EXPR,__VA_ARGS__)
#define emp_assert_impl_7(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_6(EXPR,__VA_ARGS__)
#define emp_assert_impl_8(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_7(EXPR,__VA_ARGS__)
#define emp_assert_impl_9(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_8(EXPR,__VA_ARGS__)
#define emp_assert_impl_10(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_9(EXPR,__VA_ARGS__)
#define emp_assert_impl_11(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_10(EXPR,__VA_ARGS__)
#define emp_assert_impl_12(EXPR, VAR, ...) emp_assert_var(VAR); emp_assert_impl_11(EXPR,__VA_ARGS__)

#define emp_assert(...)                                                 \
  do {                                                                  \
    std::stringstream emp_assert_var_info;                              \
    EMP_ASSEMBLE_MACRO(emp_assert_impl_, __VA_ARGS__) \
  } while(0)


#else // We ARE in DEBUG, but NOT in EMSCRIPTEN

namespace emp {
  const bool assert_on = true;
}

// Generating an output to standard error is an assert is tripped.
#define emp_assert_base_impl(EXPR)                 \
  do { if ( !(EXPR) ) {                            \
    std::cerr << "Assert Error (In " << __FILE__   \
              << " line " << __LINE__              \
              << "): " << #EXPR << std::endl;      \
    abort(); }                                     \
  } while (0)

#define emp_assert(...) emp_assert_base_impl( EMP_GET_ARG_1(__VA_ARGS__, ~) )


#endif // NDEBUG



#endif // Include guard
