#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H

#include <iostream>
#include <string>

///////////////////// Debug
#ifdef EMP_DEBUG

namespace emp {
  int assert_trip_count = 0;
  bool assert_on = true;
}

#define EMPassert(EXPR)                                                 \
  do { if ( !(EXPR) && emp::assert_trip_count++ < 3 )                   \
      emp::Alert(std::string("Assert Error (In ") + std::string(__FILE__) \
                 + std::string(" line ") + std::to_string(__LINE__)     \
                 + std::string("): ") + std::string(#EXPR)),            \
        abort();                                                        \
  } while (0)

///////////////////// NOT EMP Debug -- use normal assert rules...
#else // EMP_DEBUG

#ifdef NDEBUG
namespace emp {
  bool assert_on = false;
}

#define EMPassert(EXPR) ((void) sizeof(EXPR) )


#else // NDEBUG

#define EMPassert(EXPR)                            \
  do { if ( !(EXPR) )                              \
    std::cerr << "Assert Error (In " << __FILE__   \
              << " line " << __LINE__              \
              << "): " << #EXPR << std::endl;      \
    abort();                                       \
  } while (0)

#endif // NDEBUG

#endif // EMP_DEBUG


#endif // EMP_DEBUG_H
