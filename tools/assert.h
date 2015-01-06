#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H


///////////////////////////////////////////////////////////////////////////////////////////////
//
//  This file is a replacement for the system-level assert.h, called "emp_assert"
//  It behaves nearly identically, but provide pop-up alerts when working in a web browser.
//


#include <iostream>
#include <string>

#ifdef NDEBUG
namespace emp {
  bool assert_on = false;
}

// This assert uses the expression (to prevent compiler error), but should not
// generate any assembly code.
#define emp_assert(EXPR) ((void) sizeof(EXPR) )


#elif EMSCRIPTEN       // NDEBUG not set

namespace emp {
  int assert_trip_count = 0;
  bool assert_on = true;
}

// Generate a pop-up alert in a web browser if an assert it tripped.
#define emp_assert(EXPR)                                                \
  do { if ( !(EXPR) && emp::assert_trip_count++ < 3 ) {                 \
      emp::Alert(std::string("Assert Error (In ") + std::string(__FILE__) \
                 + std::string(" line ") + std::to_string(__LINE__)     \
                 + std::string("): ") + std::string(#EXPR)),            \
        abort(); }                                                      \
  } while (0)


#else // We ARE in DEBUG, but NOT in EMSCRIPTEN

// Generating an output to standard error is an assert is tripped.
#define emp_assert(EXPR)                           \
  do { if ( !(EXPR) ) {                            \
    std::cerr << "Assert Error (In " << __FILE__   \
              << " line " << __LINE__              \
              << "): " << #EXPR << std::endl;      \
    abort(); }                                     \
  } while (0)

#endif // NDEBUG



#endif // EMP_DEBUG_H
