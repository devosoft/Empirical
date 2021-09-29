#ifndef POLYFILL_TYPE_IDENTITY_H
#define POLYFILL_TYPE_IDENTITY_H

#if __cplusplus <= 201703L

// TODO: C++20 || cpp20
namespace std {

  // adapted from https://en.cppreference.com/w/cpp/types/type_identity
  template< class T >
  struct type_identity {
    using type = T;
  };

}

#else // #if __cplusplus <= 201703L

#include <type_traits>

#endif // #if __cplusplus <= 201703L

#endif // #ifndef POLYFILL_TYPE_IDENTITY_H
