#ifndef EMP_ASSERT_H
#define EMP_ASSERT_H

#include <string>

///////////////////// Debug
#ifdef EMP_DEBUG

namespace emp {
  int assert_trip_count = 0;
  bool assert_on = true;
}

#define assert(EXPR) if ( !(EXPR) && emp::assert_trip_count++ < 3 ) emp::Alert(std::string("Assert Error (In ") + std::string(__FILE__) + std::string(" line ") + std::to_string(__LINE__) + std::string("): ") + std::string(#EXPR))

  ///////////////////// NOT Debug
#else // EMP_DEBUG

namespace emp {
  bool assert_on = false;
}

#define assert(EXPR) ((void) 0)

#endif // EMP_DEBUG


#endif // EMP_DEBUG_H
