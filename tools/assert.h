#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H

///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is a replacement for the system-level assert.h, called "emp_assert"
//  It behaves nearly identically, but provide pop-up alerts when working in a web browser.
//
//  By default, emp_assert(X) throws an error if X evaluates to false.
//  - if NDEBUG is defined, the expression in emp_assert() is ignored (not evaluated)
//  - if TDEBUG is defined, emp_assert() is put into test mode and will record failures, but
//    not otherwise act on them.  (useful for unit tests of asserts)
//

#include <iostream>
#include <string>

// If we are in emscripten, make sure to include the header.
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif


#ifdef NDEBUG
namespace emp {
  const bool assert_on = false;
}

// This assert uses the expression (to prevent compiler error), but should not
// generate any assembly code.
// #define emp_assert(EXPR) ((void) sizeof(EXPR) )
#define emp_assert(EXPR) {                               \
    constexpr bool __emp_assert_tmp = false && (EXPR);   \
    (void) __emp_assert_tmp;                             \
  }

#elif defined(TDEBUG)           // NDEBUG not set, but TDEBUG is!

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

// Generate a pop-up alert in a web browser if an assert it tripped.
#define emp_assert(EXPR)                                                \
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



#elif EMSCRIPTEN       // NDEBUG and TDEBUG not set, but compiling with Emscripten

namespace emp {
  const bool assert_on = true;
}

// Generate a pop-up alert in a web browser if an assert it tripped.
#define emp_assert(EXPR)                                                \
  do { if ( !(EXPR) ) {                                                 \
      std::string msg = std::string("Assert Error (In ")                \
        + std::string(__FILE__)                                         \
        + std::string(" line ") + std::to_string(__LINE__)              \
        + std::string("): ") + std::string(#EXPR);                      \
      static int trip_count = 0;                                        \
      if (trip_count++ < 3)                                             \
      EM_ASM_ARGS({ msg = Pointer_stringify($0); alert(msg); }, msg.c_str()); \
      abort(); }                                                        \
  } while (0)


#else // We ARE in DEBUG, but NOT in EMSCRIPTEN

namespace emp {
  const bool assert_on = true;
}

// Generating an output to standard error is an assert is tripped.
#define emp_assert(EXPR)                           \
  do { if ( !(EXPR) ) {                            \
    std::cerr << "Assert Error (In " << __FILE__   \
              << " line " << __LINE__              \
              << "): " << #EXPR << std::endl;      \
    abort(); }                                     \
  } while (0)

#endif // NDEBUG



#endif // Include guard
