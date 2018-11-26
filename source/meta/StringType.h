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

namespace emp {

  template <typename T, size_t pos>
  struct StringPacketToIntPack {
    static constexpr auto BuildPack(T packet) {
      constexpr std::string_view text = packet();
      using recurse_t = decltype( StringPacketToIntPack<T,pos-1>::BuildPack(packet) );
      return recurse_t::push_back< (int) text[pos-1] >();
    }
  };

  template <typename T>
  struct StringPacketToIntPack<T,0> {
    static constexpr auto BuildPack(T packet) {
      return IntPack<>();
    }
  };

}

#endif
