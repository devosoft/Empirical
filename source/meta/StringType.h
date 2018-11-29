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

#include "IntPack.h"
#include "meta.h"
#include "TypeID.h"

/// Convert a literal string to an instance of an IntPack
#define EMP_TEXT_PACK(MSG) emp::StringPacketToIntPack( [](){ return MSG; } )

/// Setup a type determined by a message.
#define EMP_TEXT_TYPE(TYPE_NAME, MSG)                        \
    auto emp_temp_ ## TYPE_NAME = EMP_TEXT_PACK(MSG);        \
    using TYPE_NAME = decltype(emp_temp_ ## TYPE_NAME)
// I'd prefer for the body of EMP_TEXT_TYPE, but lambdas must be evaluated.
//    decltype(emp::StringPacketToIntPack( [](){ return MSG; } ))

/// Convert a literal string to a unique value (counting up from 0 with each string)
#define EMP_TEXT_HASH(MSG)                           \
  [](){                                              \
    constexpr auto temp = EMP_TEXT_PACK(MSG);        \
    return emp::GetTypeValue<decltype(temp)>();      \
  }()

//    constexpr auto temp = emp::StringPacketToIntPack( [](){ return MSG; } );  \

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
