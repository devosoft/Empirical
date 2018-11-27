/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  StringType.h
 *  @brief A type that maintains compile-time information about a string sequence.
 *
 */


#ifndef EMP_STRING_TYPE_H
#define EMP_STRING_TYPE_H

#include "meta.h"
#include "IntPack.h"

#define EMP_TEXT_PACKET(MSG) [](){ return MSG; }
#define EMP_TEXT_TYPE(TYPE_NAME, MSG)                                                 \
    auto emp_temp_ ## TYPE_NAME = emp::StringPacketToIntPack( [](){ return MSG; } );  \
    using TYPE_NAME = decltype(emp_temp_ ## TYPE_NAME)

//    using TYPE_NAME = decltype(emp::StringPacketToIntPack( [](){ return MSG; } ))

namespace emp {

  constexpr size_t CalcSize(const char * in) {
    size_t count = 0;
    while (in[count] != 0) count++;
    return count;
  }

  template <typename T, size_t pos>
  struct StringPacketToIntPack_impl {
    static constexpr auto BuildPack(T packet) {
      using recurse_t = decltype( StringPacketToIntPack_impl<T,pos-1>::BuildPack(packet) );
      return typename recurse_t::template push_back< (int) (packet()[pos-1]) >();
    }
  };

  template <typename T>
  struct StringPacketToIntPack_impl<T,0> {
    static constexpr auto BuildPack(T packet) {
      return IntPack<>();
    }
  };

  template <typename T>
  constexpr auto StringPacketToIntPack(T packet) {
    return emp::StringPacketToIntPack_impl<T,CalcSize(packet())>::BuildPack(packet);
  }

}

#endif
